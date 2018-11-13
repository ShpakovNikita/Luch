#include <Husky/Render/Deferred/DeferredRenderer.h>
#include <Husky/Render/TextureUploader.h>
#include <Husky/Render/ShaderDefines.h>
#include <Husky/Render/Deferred/GBufferRenderer.h>
#include <Husky/Render/Deferred/ResolveRenderer.h>
#include <Husky/Render/Deferred/TonemapRenderer.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowRenderer.h>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/AlphaMode.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Light.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/AttributeSemantic.h>

#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Graphics/TextureCreateInfo.h>
#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/ShaderLibrary.h>
#include <Husky/Graphics/DescriptorSet.h>
#include <Husky/Graphics/PhysicalDevice.h>
#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/Graphics/CommandQueue.h>
#include <Husky/Graphics/CommandPool.h>
#include <Husky/Graphics/DescriptorPool.h>
#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Graphics/Swapchain.h>
#include <Husky/Graphics/SwapchainInfo.h>
#include <Husky/Graphics/PipelineState.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/RenderPassCreateInfo.h>
#include <Husky/Graphics/DescriptorPoolCreateInfo.h>
#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Graphics/PipelineLayoutCreateInfo.h>
#include <Husky/Graphics/IndexType.h>
#include <Husky/Graphics/PipelineStateCreateInfo.h>

#include <Husky/Render/RenderUtils.h>
#include <Husky/Render/SharedBuffer.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;
    using namespace ShadowMapping;

    const String DeferredRenderer::RendererName{"Deferred"};

    DeferredRenderer::DeferredRenderer(
        const RefPtr<PhysicalDevice>& physicalDevice,
        const RefPtr<Surface>& surface,
        int32 aWidth,
        int32 aHeight)
        : width(aWidth)
        , height(aHeight)
    {
        context = MakeShared<RenderContext>();
        context->physicalDevice = physicalDevice;
        context->surface = surface;

        gbufferRenderer = MakeUnique<GBufferRenderer>();
        resolveRenderer = MakeUnique<ResolveRenderer>();
        tonemapRenderer = MakeUnique<TonemapRenderer>();
        shadowRenderer = MakeUnique<ShadowRenderer>();
    }

    DeferredRenderer::~DeferredRenderer() = default;

    bool DeferredRenderer::Initialize()
    {
        auto[createDeviceResult, createdDevice] = context->physicalDevice->CreateGraphicsDevice();
        if (createDeviceResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->device = std::move(createdDevice);

        auto& device = context->device;

        auto[createCommandQueueResult, createdCommandQueue] = device->CreateCommandQueue();
        if (createCommandQueueResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->commandQueue = std::move(createdCommandQueue);

        SwapchainInfo swapchainCreateInfo;
        swapchainCreateInfo.format = swapchainFormat;
        swapchainCreateInfo.imageCount = 1;
        swapchainCreateInfo.width = width;
        swapchainCreateInfo.height = height;

        auto[createSwapchainResult, createdSwapchain] = device->CreateSwapchain(swapchainCreateInfo, context->surface);
        if (createSwapchainResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->swapchain = std::move(createdSwapchain);

        auto [prepareResourcesResult, preparedResources] = PrepareResources();
        if(!prepareResourcesResult)
        {
            return false;
        }

        resources = std::move(preparedResources);

        gbufferRenderer->SetRenderContext(context);
        gbufferRenderer->SetDeferredResources(resources);
        bool gbufferRendererInitialized = gbufferRenderer->Initialize();
        if(!gbufferRendererInitialized)
        {
            return false;
        }

        resolveRenderer->SetRenderContext(context);
        resolveRenderer->SetDeferredResources(resources);
        bool resolveRendererInitialized = resolveRenderer->Initialize();
        if(!resolveRendererInitialized)
        {
            return false;
        }

        tonemapRenderer->SetRenderContext(context);
        bool tonemapRendererInitialized = tonemapRenderer->Initialize();
        if(!tonemapRendererInitialized)
        {
            return false;
        }

        shadowRenderer->SetRenderContext(context);
        bool shadowRendererInitialized = shadowRenderer->Initialize();
        if(!shadowRendererInitialized)
        {
            return false;
        }

        return true;
    }

    bool DeferredRenderer::Deinitialize()
    {
        resources.reset();

        bool gbufferRendererDeinitialized = gbufferRenderer->Deinitialize();
        bool shadowRendererDeinitialized = shadowRenderer->Deinitialize();
        bool resolveRendererDeinitialized = resolveRenderer->Deinitialize();
        bool tonemapRendererDeinitialized = tonemapRenderer->Deinitialize();

        return gbufferRendererDeinitialized
          && shadowRendererDeinitialized
          && tonemapRendererDeinitialized
          && resolveRendererDeinitialized;
    }

    void DeferredRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            if (node->GetCamera() != nullptr)
            {
                PrepareCameraNode(node);
            }
        }

        gbufferRenderer->PrepareScene(scene);
        resolveRenderer->PrepareScene(scene);
        shadowRenderer->PrepareScene(scene);
    }

    ResultValue<bool, UniquePtr<DeferredResources>> DeferredRenderer::PrepareResources()
    {
        auto resources = MakeUnique<DeferredResources>();

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 1;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, 1 }
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
            descriptorPoolCreateInfo);

        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources->descriptorPool = std::move(createdDescriptorPool);

        resources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources->cameraUniformBufferBinding);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            cameraDescriptorSetLayoutCreateInfo);

        if (createCameraDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources->cameraBufferDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedBufferSize;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources->sharedBuffer = MakeUnique<SharedBuffer>(std::move(createdBuffer));

        return { true, std::move(resources) };
    }

    void DeferredRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        resources->sharedBuffer->Reset();

        const auto& sceneProperties = scene->GetSceneProperties();

        for(const auto& cameraNode : sceneProperties.cameraNodes)
        {
            UpdateCamera(cameraNode->GetCamera(), cameraNode->GetWorldTransform());
        }

        gbufferRenderer->UpdateScene(scene);
        resolveRenderer->UpdateScene(scene);
        shadowRenderer->UpdateScene(scene);
    }

    void DeferredRenderer::DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera)
    {
//        const auto& lightNodesMap = scene->GetSceneProperties().lightNodes;
//        RefPtrVector<SceneV1::Node> lightNodes;
//        std::copy(lightNodesMap.begin(), lightNodesMap.end(), std::back_inserter(lightNodes));
//        auto shadowMaps = shadowRenderer->DrawShadows(scene, lightNodes);

        GBufferTextures* gbuffer = gbufferRenderer->DrawScene(scene, camera);
        Texture* resolved = resolveRenderer->Resolve(scene, camera, gbuffer);

        auto[acquireResult, acquiredTexture] = context->swapchain->GetNextAvailableTexture(nullptr);
        HUSKY_ASSERT(acquireResult == GraphicsResult::Success);

        tonemapRenderer->Tonemap(resolved, acquiredTexture.texture);

        context->commandQueue->Present(acquiredTexture.index, context->swapchain);
    }

    void DeferredRenderer::PrepareCameraNode(SceneV1::Node* node)
    {
        const auto& camera = node->GetCamera();

        auto[createVertexDescriptorSetResult, vertexDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->cameraBufferDescriptorSetLayout);
        HUSKY_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);

        camera->SetDescriptorSet(RendererName, vertexDescriptorSet);
    }

    void DeferredRenderer::UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform)
    {
        auto cameraUniform = RenderUtils::GetCameraUniform(camera, transform);
        auto descriptorSet = camera->GetDescriptorSet(RendererName);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(CameraUniform), 16);

        descriptorSet->WriteUniformBuffer(
            resources->cameraUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        descriptorSet->Update();

        memcpy(suballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));
    }
}
