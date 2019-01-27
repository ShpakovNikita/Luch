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
#include <Luch/Graphics/Semaphore.h>
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
#include <Luch/Render/Deferred/ResolveComputeRenderPass.h>
#include <Luch/Render/Deferred/ResolveComputeContext.h>
#include <Luch/Render/Deferred/TonemapRenderPass.h>
#include <Luch/Render/Deferred/TonemapContext.h>
#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphResourcePool.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Deferred;
    using namespace Graph;

    void FrameResources::Reset()
    {
        builder.reset();
        gbufferPass.reset();
        resolvePass.reset();
        resolveComputePass.reset();
        tonemapPass.reset();
        gbufferTransientContext.reset();
        resolveTransientContext.reset();
        tonemapTransientContext.reset();
        swapchainTexture.Release();
    }

    SceneRenderer::SceneRenderer(
        RefPtr<SceneV1::Scene> aScene)
        : scene(std::move(aScene))
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    bool SceneRenderer::Initialize(SharedPtr<RenderContext> aContext, SceneRendererConfig aConfig)
    {
        context = aContext;
        config = aConfig;

        auto [createCameraResourcesResult, createdCameraResources] = PrepareCameraResources(context->device);
        if(!createCameraResourcesResult)
        {
            return false;
        }

        cameraResources = std::move(createdCameraResources);

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

        auto [createSemaphoreResult, createdSemaphore] = context->device->CreateSemaphore(context->swapchain->GetInfo().imageCount);
        if(createSemaphoreResult != GraphicsResult::Success)
        {
            return false;
        }

        semaphore = std::move(createdSemaphore);

        auto [createGBufferPersistentContextResult, createdGBufferPersistentContext] = GBufferRenderPass::PrepareGBufferPersistentContext(
            context->device,
            cameraResources.get(),
            materialManager->GetResources());

        if(!createGBufferPersistentContextResult)
        {
            return false;
        }

        gbufferPersistentContext = std::move(createdGBufferPersistentContext);

        if(config.useComputeResolve)
        {
            auto [createResolvePersistentContextResult, createdResolvePersistentContext] = ResolveComputeRenderPass::PrepareResolvePersistentContext(
                context->device,
                cameraResources.get());

            if(!createResolvePersistentContextResult)
            {
                return false;
            }

            resolveComputePersistentContext = std::move(createdResolvePersistentContext);
        }
        else
        {
            auto [createResolvePersistentContextResult, createdResolvePersistentContext] = ResolveRenderPass::PrepareResolvePersistentContext(
                context->device,
                cameraResources.get());

            if(!createResolvePersistentContextResult)
            {
                return false;
            }

            resolvePersistentContext = std::move(createdResolvePersistentContext);
        }

        auto [createTonemapPersistentContextResult, createdTonemapPersistentContext] = TonemapRenderPass::PrepareTonemapPersistentContext(
            context->device,
            context->swapchain->GetInfo().format);

        if(!createTonemapPersistentContextResult)
        {
            return false;
        }

        tonemapPersistentContext = std::move(createdTonemapPersistentContext);

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

        BufferCreateInfo sharedBufferCreateInfo;
        sharedBufferCreateInfo.length = SharedBufferSize;
        sharedBufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        sharedBufferCreateInfo.usage = BufferUsageFlags::Uniform;

        for(int32 i = 0; i < MaxSwapchainTextures; i++)
        {
            auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(sharedBufferCreateInfo);
            if(createBufferResult != GraphicsResult::Success)
            {
                return false;
            }

            frameResources[i].sharedBuffer = MakeShared<SharedBuffer>(std::move(createdBuffer));

            auto [allocateCamreraDescriptorSetResult, allocatedCameraDescriptorSet] = descriptorPool->AllocateDescriptorSet(cameraResources->cameraBufferDescriptorSetLayout);
            if(allocateCamreraDescriptorSetResult != GraphicsResult::Success)
            {
                return false;
            }

            frameResources[i].cameraDescriptorSet = std::move(allocatedCameraDescriptorSet);
        }

        resourcePool = MakeUnique<RenderGraphResourcePool>(context->device);

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

    bool SceneRenderer::BeginRender()
    {
        bool timedOut = semaphore->Wait(1 * 1000 * 1000 * 1000);
        if(timedOut)
        {
            return false;
        }

        auto& frame = frameResources[GetCurrentFrameResourceIndex()];

        frame.sharedBuffer->Reset();

        frame.builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = frame.builder->Initialize(context->device, commandPool, resourcePool.get());
        if(!builderInitialized)
        {
            return false;
        }

        auto swapchainInfo = context->swapchain->GetInfo();
        Size2i outputSize = { swapchainInfo.width, swapchainInfo.height };

        outputHandle = frame.builder->GetResourceManager()->ImportTextureDeferred();

        auto [prepareGBufferTransientContextResult, preparedGBufferTransientContext] = GBufferRenderPass::PrepareGBufferTransientContext(
            gbufferPersistentContext.get(),
            descriptorPool);

        if(!prepareGBufferTransientContextResult)
        {
            return false;
        }

        frame.gbufferTransientContext = std::move(preparedGBufferTransientContext);

        frame.gbufferTransientContext->descriptorPool = descriptorPool;
        frame.gbufferTransientContext->outputSize = outputSize;
        frame.gbufferTransientContext->scene = scene;
        frame.gbufferTransientContext->sharedBuffer = frame.sharedBuffer;
        frame.gbufferTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

        frame.gbufferPass = MakeUnique<GBufferRenderPass>(
            gbufferPersistentContext.get(),
            frame.gbufferTransientContext.get(),
            frame.builder.get());

        RenderMutableResource luminanceTextureHandle;
        if(config.useComputeResolve)
        {
            auto [prepareResolveTransientContextResult, preparedResolveTransientContext] = ResolveComputeRenderPass::PrepareResolveTransientContext(
                resolveComputePersistentContext.get(),
                descriptorPool);

            if(!prepareResolveTransientContextResult)
            {
                return false;
            }

            frame.resolveComputeTransientContext = std::move(preparedResolveTransientContext);

            frame.resolveComputeTransientContext->gbuffer = frame.gbufferPass->GetGBuffer();
            frame.resolveComputeTransientContext->outputSize = outputSize;
            frame.resolveComputeTransientContext->scene = scene;
            frame.resolveComputeTransientContext->sharedBuffer = frame.sharedBuffer;
            frame.resolveComputeTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            frame.resolveComputePass = MakeUnique<ResolveComputeRenderPass>(
                resolveComputePersistentContext.get(),
                frame.resolveComputeTransientContext.get(),
                frame.builder.get());

            luminanceTextureHandle = frame.resolveComputePass->GetResolveTextureHandle();;
        }
        else
        {
            auto [prepareResolveTransientContextResult, preparedResolveTransientContext] = ResolveRenderPass::PrepareResolveTransientContext(
                resolvePersistentContext.get(),
                descriptorPool);

            if(!prepareResolveTransientContextResult)
            {
                return false;
            }

            frame.resolveTransientContext = std::move(preparedResolveTransientContext);

            frame.resolveTransientContext->gbuffer = frame.gbufferPass->GetGBuffer();
            frame.resolveTransientContext->outputSize = outputSize;
            frame.resolveTransientContext->scene = scene;
            frame.resolveTransientContext->sharedBuffer = frame.sharedBuffer;
            frame.resolveTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            frame.resolvePass = MakeUnique<ResolveRenderPass>(
                resolvePersistentContext.get(),
                frame.resolveTransientContext.get(),
                frame.builder.get());

            luminanceTextureHandle = frame.resolvePass->GetResolveTextureHandle();
        }

        LUCH_ASSERT_MSG(!config.useComputeTonemap, "Not implemented");

        auto [prepareTonemapTransientContextResult, preparedTonemapTransientContext] = TonemapRenderPass::PrepareTonemapTransientContext(
            tonemapPersistentContext.get(),
            descriptorPool);

        if(!prepareTonemapTransientContextResult)
        {
            return false;
        }

        frame.tonemapTransientContext = std::move(preparedTonemapTransientContext);

        frame.tonemapTransientContext->inputHandle = luminanceTextureHandle;
        frame.tonemapTransientContext->outputHandle = outputHandle;
        frame.tonemapTransientContext->outputSize = outputSize;
        frame.tonemapTransientContext->scene = scene;

        frame.tonemapPass = MakeUnique<TonemapRenderPass>(
            tonemapPersistentContext.get(),
            frame.tonemapTransientContext.get(),
            frame.builder.get());

        return true;
    }

    bool SceneRenderer::PrepareScene()
    {
        auto& frame = frameResources[GetCurrentFrameResourceIndex()];

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

        frame.gbufferPass->PrepareScene();
        frame.resolvePass->PrepareScene();
        frame.tonemapPass->PrepareScene();

        return true;
    }

    void SceneRenderer::UpdateScene()
    {
        auto& frame = frameResources[GetCurrentFrameResourceIndex()];

        auto& materials = scene->GetSceneProperties().materials;

        for(auto& material : materials)
        {
            materialManager->UpdateMaterial(material, frame.sharedBuffer.get());
        }

        frame.gbufferPass->UpdateScene();
        if(config.useComputeResolve)
        {
            frame.resolveComputePass->UpdateScene();
        }
        else
        {
            frame.resolvePass->UpdateScene();
        }
        frame.tonemapPass->UpdateScene();
    }

    void SceneRenderer::DrawScene(SceneV1::Node* cameraNode)
    {
        auto camera = cameraNode->GetCamera();
        LUCH_ASSERT(camera != nullptr);

        auto& frame = frameResources[GetCurrentFrameResourceIndex()];

        CameraUniform cameraUniform = RenderUtils::GetCameraUniform(camera, cameraNode->GetWorldTransform());
        auto suballocation = frame.sharedBuffer->Suballocate(sizeof(CameraUniform), 256);

        memcpy(suballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));

        frame.cameraDescriptorSet->WriteUniformBuffer(
            cameraResources->cameraUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        frame.cameraDescriptorSet->Update();

        auto [getNextTextureResult, swapchainTexture] = context->swapchain->GetNextAvailableTexture();
        LUCH_ASSERT(getNextTextureResult == GraphicsResult::Success);

        frame.swapchainTexture = swapchainTexture;

        frame.builder->GetResourceManager()->ProvideDeferredTexture(outputHandle, swapchainTexture->GetTexture());

        auto [buildResult, renderGraph] = frame.builder->Build();
        auto commandLists = renderGraph->Execute();
        for(auto& commandList : commandLists)
        {
            context->commandQueue->Submit(commandList);
        }
    }

    void SceneRenderer::EndRender()
    {
        auto index = GetCurrentFrameResourceIndex();

        context->commandQueue->Present(
            frameResources[index].swapchainTexture,
            [this, index]()
            {
                frameResources[index].Reset();
                resourcePool->Tick();
                semaphore->Signal();
            });

        frameIndex++;
    }

    int32 SceneRenderer::GetCurrentFrameResourceIndex() const
    {
        auto swapchainImageCount = context->swapchain->GetInfo().imageCount;
        return frameIndex % swapchainImageCount;
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
        SceneV1::SceneProperties::Usages<SceneV1::Texture> textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            if(texture->GetDeviceTexture() != nullptr && texture->GetDeviceSampler() != nullptr)
            {
                continue;
            }

            texturesVector.push_back(texture);
        }

        if(texturesVector.empty())
        {
            return true;
        }

        auto [createCommandPoolResult, commandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        TextureUploader textureUploader{ context->device, commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector, true);

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
        SceneV1::SceneProperties::Usages<SceneV1::Buffer> buffers = sceneProperties.buffers;
    
        if(buffers.empty())
        {
            return false;
        }

        for(const auto& buffer : buffers)
        {
            if(buffer->IsResidentOnDevice())
            {
                continue;
            }

            [[maybe_unused]] bool uploadSucceeded = buffer->UploadToDevice(context->device);
            if(!uploadSucceeded)
            {
                return false;
            }
        }

        return true;
    }
}
