#include <Luch/Render/SceneRenderer.h>
#include <Luch/Render/TextureUploader.h>
#include <Luch/Render/ShaderDefines.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/AttributeSemantic.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/PipelineState.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/Graphics/PipelineStateCreateInfo.h>

#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>

namespace Luch::Render
{
    using namespace Graphics;

    SceneRenderer::SceneRenderer(
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
    }

    SceneRenderer::~SceneRenderer() = default;

    bool SceneRenderer::Initialize()
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

        return true;
    }

    bool SceneRenderer::Deinitialize()
    {
        return true;
    }

    void SceneRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            if (node->GetCamera() != nullptr)
            {
                PrepareCameraNode(node);
            }
        }

        // gbufferRenderer->PrepareScene(scene);
        // resolveRenderer->PrepareScene(scene);
        // shadowRenderer->PrepareScene(scene);
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
            LUCH_ASSERT(false);
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
            LUCH_ASSERT(false);
            return { false };
        }

        resources->cameraBufferDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedBufferSize;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
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
        LUCH_ASSERT(acquireResult == GraphicsResult::Success);

        tonemapRenderer->Tonemap(resolved, acquiredTexture.texture);

        context->commandQueue->Present(acquiredTexture.index, context->swapchain);
    }

    void DeferredRenderer::PrepareCameraNode(SceneV1::Node* node)
    {
        const auto& camera = node->GetCamera();

        auto[createVertexDescriptorSetResult, vertexDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->cameraBufferDescriptorSetLayout);
        LUCH_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);

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
