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

#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>

#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Deferred/GBufferRenderPass.h>
#include <Luch/Render/Deferred/GBufferContext.h>
#include <Luch/Render/Deferred/ResolveRenderPass.h>
#include <Luch/Render/Deferred/ResolveContext.h>
#include <Luch/Render/Deferred/TonemapRenderPass.h>
#include <Luch/Render/Deferred/TonemapContext.h>
#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Deferred;
    using namespace Graph;

    SceneRenderer::SceneRenderer(
        RefPtr<SceneV1::Scene> aScene)
        : scene(std::move(aScene))
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    bool SceneRenderer::Initialize(SharedPtr<RenderContext> aContext)
    {
        context = aContext;
        materialManager = MakeUnique<MaterialManager>();

        auto materialManagerInitialized = materialManager->Initialize(context->device);

        if(!materialManagerInitialized)
        {
            return false;
        }

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        commandPool = std::move(createdCommandPool);

        auto [createGBufferPersistentContextResult, createdGBufferPersistentContext] = GBufferRenderPass::PrepareGBufferPersistentContext(
            context->device,
            cameraResources.get(),
            materialManager->GetResources());

        if(!createGBufferPersistentContextResult)
        {
            return false;
        }

        gbufferPersistentContext = std::move(createdGBufferPersistentContext);

        auto [createResolvePersistentContextResult, createdResolvePersistentContext] = ResolveRenderPass::PrepareResolvePersistentContext(
            context->device,
            cameraResources.get());

        if(!createResolvePersistentContextResult)
        {
            return false;
        }

        resolvePersistentContext = std::move(createdResolvePersistentContext);

        auto [createTonemapPersistentContextResult, createdTonemapPersistentContext] = TonemapRenderPass::PrepareTonemapPersistentContext(
            context->device,
            context->swapchain->GetInfo().format);

        if(!createTonemapPersistentContextResult)
        {
            return false;
        }

        tonemapPersistentContext = std::move(createdTonemapPersistentContext);

        auto [createCameraResourcesResult, createdCameraResources] = PrepareCameraResources(context->device);
        if(!createCameraResourcesResult)
        {
            return false;
        }

        cameraResources = std::move(createdCameraResources);

        BufferCreateInfo sharedBufferCreateInfo;
        sharedBufferCreateInfo.length = SharedBufferSize;
        sharedBufferCreateInfo.storageMode = ResourceStorageMode::DeviceLocal;
        sharedBufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(sharedBufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            return false;
        }

        sharedBuffer = MakeShared<SharedBuffer>(std::move(createdBuffer));

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = DescriptorSetCount;
        descriptorPoolCreateInfo.descriptorCount = 
        {
            { ResourceType::Texture, DescriptorCount },
            { ResourceType::Sampler, DescriptorCount },
            { ResourceType::UniformBuffer, DescriptorCount },
        };

        auto [createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if(createDescriptorPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        descriptorPool = std::move(createdDescriptorPool);

        return true;
    }

    bool SceneRenderer::Deinitialize()
    {
        commandPool.Release();
        tonemapPersistentContext.reset();
        resolvePersistentContext.reset();
        gbufferPersistentContext.reset();
        context.reset();

        auto materialManagerDeinitialized = materialManager->Deinitialize();
        if(!materialManagerDeinitialized)
        {
            return false;
        }

        return true;
    }

    void SceneRenderer::BeginRender()
    {
        auto swapchainInfo = context->swapchain->GetInfo();
        Size2i attachmentSize = { swapchainInfo.width, swapchainInfo.height };

        auto gbufferTransientContext = MakeUnique<GBufferTransientContext>();
        gbufferTransientContext->descriptorPool = descriptorPool;
        gbufferTransientContext->attachmentSize = attachmentSize;
        gbufferTransientContext->scene = scene;
        gbufferTransientContext->sharedBuffer = sharedBuffer;

        auto [createResolveTransientContextResult, resolveTransientContext] = ResolveRenderPass::PrepareResolveTransientContext(
            resolvePersistentContext.get(),
            descriptorPool);

        LUCH_ASSERT(createResolveTransientContextResult);

        resolveTransientContext->attachmentSize = attachmentSize;
        resolveTransientContext->scene = scene;
        resolveTransientContext->sharedBuffer = sharedBuffer;

        auto [createTonemapTransientContextResult, createdTonemapTransientContext] = TonemapRenderPass::PrepareTonemapTransientContext(
            tonemapPersistentContext.get(),
            descriptorPool);

        LUCH_ASSERT(createTonemapTransientContextResult);

        tonemapTransientContext->attachmentSize = attachmentSize;
        tonemapTransientContext->scene = scene;

        builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = builder->Initialize(context->device, commandPool);
        LUCH_ASSERT(builderInitialized);

        gbufferPass = MakeUnique<GBufferRenderPass>(
            gbufferPersistentContext.get(),
            gbufferTransientContext.get(),
            builder.get());

        resolvePass = MakeUnique<ResolveRenderPass>(
            resolvePersistentContext.get(),
            resolveTransientContext.get(),
            builder.get());

        tonemapPass = MakeUnique<TonemapRenderPass>(
            tonemapPersistentContext.get(),
            tonemapTransientContext.get(),
            builder.get());
    }

    bool SceneRenderer::PrepareScene()
    {
        auto texturesUploaded = UploadSceneTextures();
        if(!texturesUploaded)
        {
            return false;
        }

        auto buffersUploaded = UploadSceneBuffers();
        if(!buffersUploaded)
        {
            return false;
        }

        auto& materials = scene->GetSceneProperties().materials;

        for(auto& material : materials)
        {
            auto materialPrepared = materialManager->PrepareMaterial(material);
            if(!materialPrepared)
            {
                return false;
            }
        }

        gbufferPass->PrepareScene();
        resolvePass->PrepareScene();
        tonemapPass->PrepareScene();

        return true;
    }

    void SceneRenderer::UpdateScene()
    {
        auto& materials = scene->GetSceneProperties().materials;

        for(auto& material : materials)
        {
            materialManager->UpdateMaterial(material);
        }

        gbufferPass->UpdateScene();
        resolvePass->UpdateScene();
        tonemapPass->UpdateScene();
    }

    void SceneRenderer::DrawScene(SceneV1::Camera* camera)
    {
        auto [buildResult, renderGraph] = builder->Build();
        auto commandLists = renderGraph->Execute();
        for(auto& commandList : commandLists)
        {
            context->commandQueue->Submit(commandList);
        }
    }

    void SceneRenderer::EndRender()
    {
        builder.reset();
        gbufferTransientContext.reset();
        resolveTransientContext.reset();
        tonemapTransientContext.reset();
        gbufferPass.reset();
        resolvePass.reset();
        tonemapPass.reset();
    }

    ResultValue<bool, UniquePtr<CameraResources>> SceneRenderer::PrepareCameraResources(GraphicsDevice* device)
    {
        UniquePtr<CameraResources> cameraResources = MakeUnique<CameraResources>();

        cameraResources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo
            .AddBinding(&cameraResources->cameraUniformBufferBinding)
            .OfType(DescriptorSetType::Buffer);

        auto [createDescriptorSetLayoutResult, createdDescriptorSetLayout] = device->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);
        if(createDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            return { false };
        }

        cameraResources->cameraBufferDescriptorSetLayout = std::move(createdDescriptorSetLayout);

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 10;
        descriptorPoolCreateInfo.descriptorCount = 
        {
            { ResourceType::UniformBuffer, 10}
        };

        auto [createDescriptorPoolResult, createdDescriptorPool] = device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if(createDescriptorPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        cameraResources->descriptorPool = std::move(createdDescriptorPool);

        return { true, std::move(cameraResources) };
    }

    bool SceneRenderer::UploadSceneTextures()
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        const auto& textures = sceneProperties.textures;

        auto [createCommandPoolResult, commandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            texturesVector.push_back(texture);
        }

        TextureUploader textureUploader{ context->device, commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);

        if(!uploadTexturesSucceeded)
        {
            return false;
        }

        for (const auto& commandList : uploadTexturesResult.commandLists)
        {
            context->commandQueue->Submit(commandList);
        }

        return true;
    }

    bool SceneRenderer::UploadSceneBuffers()
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        const auto& buffers = sceneProperties.buffers;

        for(const auto& buffer : buffers)
        {
            [[maybe_unused]] bool uploadSucceeded = buffer->UploadToDevice(context->device);
            if(!uploadSucceeded)
            {
                return false;
            }
        }

        return true;
    }
}
