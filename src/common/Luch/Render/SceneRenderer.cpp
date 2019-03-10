#include <Luch/Render/SceneRenderer.h>
#include <Luch/Render/TextureUploader.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/LightProbe.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/AttributeSemantic.h>

#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/PhysicalDeviceCapabilities.h>
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
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/IndirectLightingResources.h>
#include <Luch/Render/IBLRenderer.h>

#include <Luch/Render/Passes/DepthOnlyRenderPass.h>
#include <Luch/Render/Passes/DepthOnlyContext.h>

#include <Luch/Render/Passes/Forward/ForwardRenderPass.h>
#include <Luch/Render/Passes/Forward/ForwardContext.h>

#include <Luch/Render/Passes/TonemapRenderPass.h>
#include <Luch/Render/Passes/TonemapContext.h>

#include <Luch/Render/Passes/Deferred/GBufferRenderPass.h>
#include <Luch/Render/Passes/Deferred/GBufferContext.h>

#include <Luch/Render/Passes/Deferred/ResolveRenderPass.h>
#include <Luch/Render/Passes/Deferred/ResolveContext.h>

#include <Luch/Render/Passes/Deferred/ResolveComputeRenderPass.h>
#include <Luch/Render/Passes/Deferred/ResolveComputeContext.h>

#include <Luch/Render/Passes/TiledDeferred/TiledDeferredContext.h>
#include <Luch/Render/Passes/TiledDeferred/TiledDeferredRenderPass.h>

#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphResourcePool.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Passes;
    using namespace Deferred;
    using namespace Forward;
    using namespace TiledDeferred;
    using namespace IBL;
    using namespace Graph;

    void FrameResources::Reset()
    {
        builder.reset();
        renderGraph.reset();
        depthOnlyPass.reset();
        tiledDeferredPass.reset();
        gbufferPass.reset();
        resolvePass.reset();
        resolveComputePass.reset();
        tonemapPass.reset();
        depthOnlyTransientContext.reset();
        tiledDeferredTransientContext.reset();
        gbufferTransientContext.reset();
        resolveTransientContext.reset();
        tonemapTransientContext.reset();
        swapchainTexture.Release();

        diffuseIrradianceCubemapHandle = nullptr;
        specularReflectionCubemapHandle = nullptr;
        specularBRDFTextureHandle = nullptr;
    }

    SceneRenderer::SceneRenderer(
        RefPtr<SceneV1::Scene> aScene)
        : scene(std::move(aScene))
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    bool SceneRenderer::Initialize(SharedPtr<RenderContext> aContext)
    {
        context = aContext;

        canUseTiledDeferredRender = context->device->GetPhysicalDevice()->GetCapabilities().hasTileBasedArchitecture;

        {
            auto [result, createdCameraResources] = PrepareCameraResources(context->device);
            if(!result)
            {
                return false;
            }

            cameraResources = std::move(createdCameraResources);
        }

        {
            auto [result, createdIndirectLightingResources] = PrepareIndirectLightingResources(context->device);
            if(!result)
            {
                return false;
            }

            indirectLightingResources = std::move(createdIndirectLightingResources);
        }

        materialManager = MakeShared<MaterialManager>();

        auto materialManagerInitialized = materialManager->Initialize(context->device);

        if(!materialManagerInitialized)
        {
            return false;
        }

        iblRenderer = MakeUnique<IBLRenderer>(scene);

        bool iblRendererInitialized = iblRenderer->Initialize(context, materialManager, cameraResources);
        if(!iblRendererInitialized)
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

        // Depth-only Persistent Context
        {
            auto [createDepthOnlyPersistentContextResult, createdDepthOnlyPersistentContext] = DepthOnlyRenderPass::PrepareDepthOnlyPersistentContext(
                context->device,
                cameraResources.get(),
                materialManager->GetResources());

            if(!createDepthOnlyPersistentContextResult)
            {
                return false;
            }

            depthOnlyPersistentContext = std::move(createdDepthOnlyPersistentContext);
        }

        // Forward Persistent Context
        {
            auto [createForwardPersistentContextResult, createdForwardPersistentContext] = ForwardRenderPass::PrepareForwardPersistentContext(
                context->device,
                cameraResources.get(),
                materialManager->GetResources(),
                indirectLightingResources.get());
            
            if(!createForwardPersistentContextResult)
            {
                return false;
            }

            forwardPersistentContext = std::move(createdForwardPersistentContext);
        }

        // Tiled Deferred Persistent Context
        if(canUseTiledDeferredRender)
        {
            auto [createTiledDeferredPersistentContextResult, createdTiledDeferredPersistentContext] = TiledDeferredRenderPass::PrepareTiledDeferredPersistentContext(
                context->device,
                cameraResources.get(),
                materialManager->GetResources());

            if(!createTiledDeferredPersistentContextResult)
            {
                return false;
            }

            tiledDeferredPersistentContext = std::move(createdTiledDeferredPersistentContext);
        }

        // GBuffer Persistent Context
        auto [createGBufferPersistentContextResult, createdGBufferPersistentContext] = GBufferRenderPass::PrepareGBufferPersistentContext(
            context->device,
            cameraResources.get(),
            materialManager->GetResources());

        if(!createGBufferPersistentContextResult)
        {
            return false;
        }

        gbufferPersistentContext = std::move(createdGBufferPersistentContext);

        // Resolve (Compute) Persistent Context
        auto [createResolveComputePersistentContextResult, createdResolveComputePersistentContext] = ResolveComputeRenderPass::PrepareResolvePersistentContext(
            context->device,
            cameraResources.get(),
            indirectLightingResources.get());

        if(!createResolveComputePersistentContextResult)
        {
            return false;
        }

        resolveComputePersistentContext = std::move(createdResolveComputePersistentContext);

        // Resolve (Graphics) Persistent Context
        auto [createResolvePersistentContextResult, createdResolvePersistentContext] = ResolveRenderPass::PrepareResolvePersistentContext(
            context->device,
            cameraResources.get(),
            indirectLightingResources.get());

        if(!createResolvePersistentContextResult)
        {
            return false;
        }

        resolvePersistentContext = std::move(createdResolvePersistentContext);

        // Tonemap Persistent Context
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
        depthOnlyPersistentContext.reset();
        tiledDeferredPersistentContext.reset();
        tonemapPersistentContext.reset();
        resolvePersistentContext.reset();
        gbufferPersistentContext.reset();
        context.reset();

        auto materialManagerDeinitialized = materialManager->Deinitialize();
        if(!materialManagerDeinitialized)
        {
            return false;
        }

        auto iblRendererDeinitialized = iblRenderer->Deinitialize();
        if(!iblRendererDeinitialized)
        {
            return false;
        }

        return true;
    }

    bool SceneRenderer::ProbeIndirectLighting()
    {
        if(!config.useGlobalIllumination)
        {
            return true;
        }

        auto lightProbeNodeIt = std::find_if(
            scene->GetNodes().begin(),
            scene->GetNodes().end(),
            [](const auto& node) { return node->GetLightProbe() != nullptr && node->GetLightProbe()->IsEnabled(); });

        if(lightProbeNodeIt == scene->GetNodes().end())
        {
            return true;
        }

        const auto& lightProbeNode = *lightProbeNodeIt;
        const auto& lightProbe = lightProbeNode->GetLightProbe();

        IBLRequest iblRequest;
        iblRequest.position = lightProbeNode->GetWorldTransform() * Vec4{ 0, 0, 0, 1 };
        iblRequest.probeDiffuseIrradiance = lightProbe->HasDiffuseIrradiance();
        iblRequest.probeSpecularReflection = lightProbe->HasSpecularReflection();
        iblRequest.computeSpecularBRDF = true;
        iblRequest.size = lightProbe->GetSize();
        iblRequest.zNear = lightProbe->GetZNear();
        iblRequest.zFar = lightProbe->GetZFar();

        bool began = iblRenderer->BeginRender(iblRequest);
        if(!began)
        {
            return false;
        }

        bool prepared = iblRenderer->PrepareScene();
        if(!prepared)
        {
            return false;
        }

        iblRenderer->UpdateScene();

        iblRenderer->ProbeIndirectLighting();

        auto [result, probe] = iblRenderer->EndRender();
        if(!result)
        {
            return false;
        }

        diffuseIrradianceCubemap = probe.diffuseIrradianceCubemap;
        specularReflectionCubemap = probe.specularReflectionCubemap;
        specularBRDFTexture = probe.specularBRDFTexture;

        return true;
    }

    void SceneRenderer::ResetIndirectLighting()
    {
        diffuseIrradianceCubemap = nullptr;
        specularReflectionCubemap = nullptr;
        specularBRDFTexture = nullptr;
    }

    bool SceneRenderer::BeginRender()
    {
        bool timedOut = semaphore->Wait();
        if(timedOut)
        {
            return false;
        }

        auto& frame = frameResources[GetCurrentFrameResourceIndex()];

        frame.Reset();

        frame.sharedBuffer->Reset();

        frame.builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = frame.builder->Initialize(context->device, commandPool, resourcePool.get());
        if(!builderInitialized)
        {
            return false;
        }

        auto swapchainInfo = context->swapchain->GetInfo();
        frame.outputSize = { swapchainInfo.width, swapchainInfo.height };

        frame.outputHandle = frame.builder->GetResourceManager()->ImportTextureDeferred();

        RenderMutableResource luminanceTextureHandle;

        LUCH_ASSERT(!(config.useDepthPrepass && config.useTiledDeferredPass));
        LUCH_ASSERT(!(config.useComputeResolve && config.useTiledDeferredPass));
        LUCH_ASSERT(!(config.useForward && config.useTiledDeferredPass));

        if(config.useDiffuseGlobalIllumination)
        {
            frame.diffuseIrradianceCubemapHandle = frame.builder->GetResourceManager()->ImportTexture(diffuseIrradianceCubemap);
        }

        if(config.useSpecularGlobalIllumination)
        {
            frame.specularReflectionCubemapHandle = frame.builder->GetResourceManager()->ImportTexture(specularReflectionCubemap);
            frame.specularBRDFTextureHandle = frame.builder->GetResourceManager()->ImportTexture(specularBRDFTexture);
        }

        if(config.useDepthPrepass)
        {
            auto [prepareDepthOnlyTransientContextResult, preparedDepthOnlyTransientContext] = DepthOnlyRenderPass::PrepareDepthOnlyTransientContext(
                depthOnlyPersistentContext.get(),
                descriptorPool);

            if(!prepareDepthOnlyTransientContextResult)
            {
                return false;
            }

            frame.depthOnlyTransientContext = std::move(preparedDepthOnlyTransientContext);

            frame.depthOnlyTransientContext->descriptorPool = descriptorPool;
            frame.depthOnlyTransientContext->outputSize = frame.outputSize;
            frame.depthOnlyTransientContext->scene = scene;
            frame.depthOnlyTransientContext->sharedBuffer = frame.sharedBuffer;
            frame.depthOnlyTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            frame.depthOnlyPass = MakeUnique<DepthOnlyRenderPass>(
                depthOnlyPersistentContext.get(),
                frame.depthOnlyTransientContext.get(),
                frame.builder.get());
        }

        if(config.useForward)
        {
            bool forwardPrepared = PrepareForward(frame);
            LUCH_ASSERT(forwardPrepared);
            if(!forwardPrepared)
            {
                return false;
            }

            luminanceTextureHandle = frame.forwardPass->GetLuminanceTextureHandle();
        }
        else if(config.useTiledDeferredPass && canUseTiledDeferredRender)
        {
            bool tileDeferredPrepared = PrepareTiledDeferred(frame);
            LUCH_ASSERT(tileDeferredPrepared);
            if(!tileDeferredPrepared)
            {
                return false;
            }

            luminanceTextureHandle = frame.tiledDeferredPass->GetLuminanceTextureHandle();
        }
        else
        {
            bool deferredPrepared = PrepareDeferred(frame);
            LUCH_ASSERT(deferredPrepared);
            if(!deferredPrepared)
            {
                return false;
            }

            if(config.useComputeResolve)
            {
                luminanceTextureHandle = frame.resolveComputePass->GetLuminanceTextureHandle();
            }
            else
            {
                luminanceTextureHandle = frame.resolvePass->GetLuminanceTextureHandle();
            }
        }

        {
            auto [prepareTonemapTransientContextResult, preparedTonemapTransientContext] = TonemapRenderPass::PrepareTonemapTransientContext(
                tonemapPersistentContext.get(),
                descriptorPool);

            if(!prepareTonemapTransientContextResult)
            {
                return false;
            }

            frame.tonemapTransientContext = std::move(preparedTonemapTransientContext);

            frame.tonemapTransientContext->inputHandle = luminanceTextureHandle;
            frame.tonemapTransientContext->outputHandle = frame.outputHandle;
            frame.tonemapTransientContext->outputSize = frame.outputSize;
            frame.tonemapTransientContext->scene = scene;

            frame.tonemapPass = MakeUnique<TonemapRenderPass>(
                tonemapPersistentContext.get(),
                frame.tonemapTransientContext.get(),
                frame.builder.get());
        }

        return true;
    }

    bool SceneRenderer::PrepareSceneResources()
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

        return true;
    }

    bool SceneRenderer::PrepareScene()
    {
        auto& frame = frameResources[GetCurrentFrameResourceIndex()];

        if(config.useDepthPrepass)
        {
            frame.depthOnlyPass->PrepareScene();
        }

        if(config.useForward)
        {
            frame.forwardPass->PrepareScene();
        }
        else if(config.useTiledDeferredPass)
        {
            frame.tiledDeferredPass->PrepareScene();
        }
        else
        {
            frame.gbufferPass->PrepareScene();

            if(config.useComputeResolve)
            {
                frame.resolveComputePass->PrepareScene();
            }
            else
            {
                frame.resolvePass->PrepareScene();
            }
        }

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

        if(config.useDepthPrepass)
        {
            frame.depthOnlyPass->UpdateScene();
        }

        if(config.useForward)
        {
            frame.forwardPass->UpdateScene();
        }
        else if(config.useTiledDeferredPass)
        {
            frame.tiledDeferredPass->UpdateScene();
        }
        else
        {
            frame.gbufferPass->UpdateScene();

            if(config.useComputeResolve)
            {
                frame.resolveComputePass->UpdateScene();
            }
            else
            {
                frame.resolvePass->UpdateScene();
            }
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

        frame.builder->GetResourceManager()->ProvideDeferredTexture(frame.outputHandle, swapchainTexture->GetTexture());

        auto [buildResult, builtRenderGraph] = frame.builder->Build();
        frame.renderGraph = std::move(builtRenderGraph);

        auto commandLists = frame.renderGraph->Execute();
        RenderUtils::SubmitCommandLists(context->commandQueue, commandLists);
    }

    void SceneRenderer::EndRender()
    {
        auto index = GetCurrentFrameResourceIndex();

        context->commandQueue->Present(
            frameResources[index].swapchainTexture,
            [this, index]()
            {
                resourcePool->Tick();
                semaphore->Signal();
            });

        frameIndex++;
    }

    bool SceneRenderer::PrepareForward(FrameResources& frame)
    {
        auto [prepareForwardTransientContextResult, preparedForwardTransientContext] = ForwardRenderPass::PrepareForwardTransientContext(
            forwardPersistentContext.get(),
            descriptorPool);

        if(!prepareForwardTransientContextResult)
        {
            return false;
        }

        frame.forwardTransientContext = std::move(preparedForwardTransientContext);

        frame.forwardTransientContext->descriptorPool = descriptorPool;
        frame.forwardTransientContext->outputSize = frame.outputSize;
        frame.forwardTransientContext->scene = scene;
        frame.forwardTransientContext->sharedBuffer = frame.sharedBuffer;
        frame.forwardTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

        if(config.useDiffuseGlobalIllumination)
        {
            frame.forwardTransientContext->diffuseIrradianceCubemapHandle = frame.diffuseIrradianceCubemapHandle;
        }

        if(config.useSpecularGlobalIllumination)
        {
            frame.forwardTransientContext->specularReflectionCubemapHandle = frame.specularReflectionCubemapHandle;
            frame.forwardTransientContext->specularBRDFTextureHandle = frame.specularBRDFTextureHandle;
        }

        if(config.useDepthPrepass)
        {
            frame.forwardTransientContext->useDepthPrepass = true;
            frame.forwardTransientContext->depthStencilTextureHandle = frame.depthOnlyPass->GetDepthTextureHandle();
        }

        frame.forwardPass = MakeUnique<ForwardRenderPass>(
            forwardPersistentContext.get(),
            frame.forwardTransientContext.get(),
            frame.builder.get());

        return true;
    }

    bool SceneRenderer::PrepareDeferred(FrameResources& frame)
    {
        {
            auto [contextPrepared, transientContext] = GBufferRenderPass::PrepareGBufferTransientContext(
                gbufferPersistentContext.get(),
                descriptorPool);

            if(!contextPrepared)
            {
                return false;
            }

            transientContext->descriptorPool = descriptorPool;
            transientContext->outputSize = frame.outputSize;
            transientContext->scene = scene;
            transientContext->sharedBuffer = frame.sharedBuffer;
            transientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            if(config.useDepthPrepass)
            {
                transientContext->useDepthPrepass = true;
                transientContext->depthStencilTextureHandle = frame.depthOnlyPass->GetDepthTextureHandle();
            }

            frame.gbufferTransientContext = std::move(transientContext);

            frame.gbufferPass = MakeUnique<GBufferRenderPass>(
                gbufferPersistentContext.get(),
                frame.gbufferTransientContext.get(),
                frame.builder.get());
        }

        if(config.useComputeResolve)
        {
            auto [contextPrepared, transientContext] = ResolveComputeRenderPass::PrepareResolveTransientContext(
                resolveComputePersistentContext.get(),
                descriptorPool);

            if(!contextPrepared)
            {
                return false;
            }

            transientContext->gbuffer = frame.gbufferPass->GetGBuffer();
            transientContext->outputSize = frame.outputSize;
            transientContext->scene = scene;
            transientContext->sharedBuffer = frame.sharedBuffer;
            transientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            if(config.useDiffuseGlobalIllumination)
            {
                transientContext->diffuseIrradianceCubemapHandle = frame.diffuseIrradianceCubemapHandle;
            }

            if(config.useSpecularGlobalIllumination)
            {
                transientContext->specularReflectionCubemapHandle = frame.specularReflectionCubemapHandle;
                transientContext->specularBRDFTextureHandle = frame.specularBRDFTextureHandle;
            }

            frame.resolveComputeTransientContext = std::move(transientContext);

            frame.resolveComputePass = MakeUnique<ResolveComputeRenderPass>(
                resolveComputePersistentContext.get(),
                frame.resolveComputeTransientContext.get(),
                frame.builder.get());
        }
        else
        {
            auto [contextPrepared, transientContext] = ResolveRenderPass::PrepareResolveTransientContext(
                resolvePersistentContext.get(),
                descriptorPool);

            if(!contextPrepared)
            {
                return false;
            }

            transientContext->gbuffer = frame.gbufferPass->GetGBuffer();
            transientContext->outputSize = frame.outputSize;
            transientContext->scene = scene;
            transientContext->sharedBuffer = frame.sharedBuffer;
            transientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            if(config.useDiffuseGlobalIllumination)
            {
                transientContext->diffuseIrradianceCubemapHandle = frame.diffuseIrradianceCubemapHandle;
            }

            if(config.useSpecularGlobalIllumination)
            {
                transientContext->specularReflectionCubemapHandle = frame.specularReflectionCubemapHandle;
                transientContext->specularBRDFTextureHandle = frame.specularBRDFTextureHandle;
            }

            frame.resolveTransientContext = std::move(transientContext);

            frame.resolvePass = MakeUnique<ResolveRenderPass>(
                resolvePersistentContext.get(),
                frame.resolveTransientContext.get(),
                frame.builder.get());
        }

        return true;
    }

    bool SceneRenderer::PrepareTiledDeferred(FrameResources& frame)
    {
        auto [prepareTiledDeferredTransientContextResult, preparedTiledDeferredTransientContext] = TiledDeferredRenderPass::PrepareTiledDeferredTransientContext(
            tiledDeferredPersistentContext.get(),
            descriptorPool);

        if(!prepareTiledDeferredTransientContextResult)
        {
            LUCH_ASSERT(false);
            return false;
        }

        frame.tiledDeferredTransientContext = std::move(preparedTiledDeferredTransientContext);

        frame.tiledDeferredTransientContext->descriptorPool = descriptorPool;
        frame.tiledDeferredTransientContext->outputSize = frame.outputSize;
        frame.tiledDeferredTransientContext->scene = scene;
        frame.tiledDeferredTransientContext->sharedBuffer = frame.sharedBuffer;
        frame.tiledDeferredTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

        frame.tiledDeferredPass = MakeUnique<TiledDeferredRenderPass>(
            tiledDeferredPersistentContext.get(),
            frame.tiledDeferredTransientContext.get(),
            frame.builder.get());

        return true;
    }


    int32 SceneRenderer::GetCurrentFrameResourceIndex() const
    {
        auto swapchainImageCount = context->swapchain->GetInfo().imageCount;
        return frameIndex % swapchainImageCount;
    }

    ResultValue<bool, UniquePtr<CameraResources>> SceneRenderer::PrepareCameraResources(
        GraphicsDevice* device)
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

    ResultValue<bool, UniquePtr<IndirectLightingResources>> SceneRenderer::PrepareIndirectLightingResources(
        GraphicsDevice* device)
    {
        auto resources = MakeUnique<IndirectLightingResources>();

        resources->diffuseIrradianceCubemapBinding.OfType(ResourceType::Texture);
        resources->specularReflectionCubemapBinding.OfType(ResourceType::Texture);
        resources->specularBRDFTextureBinding.OfType(ResourceType::Texture);

        DescriptorSetLayoutCreateInfo createInfo;
        createInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(3)
            .AddBinding(&resources->diffuseIrradianceCubemapBinding)
            .AddBinding(&resources->specularReflectionCubemapBinding)
            .AddBinding(&resources->specularBRDFTextureBinding);

        auto[result, descriptorSetLayout] = device->CreateDescriptorSetLayout(createInfo);
        if (result != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->indirectLightingTexturesDescriptorSetLayout = std::move(descriptorSetLayout);

        return { true, std::move(resources) };
    }

    bool SceneRenderer::UploadSceneTextures()
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        SceneV1::SceneProperties::Usages<SceneV1::Texture> textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            if(texture->GetHostImage() == nullptr)
            {
                continue;
            }

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

        RenderUtils::SubmitCommandLists(context->commandQueue, uploadTexturesResult.commandLists);

        // Release uploaded host images
        for (const auto& texture : textures)
        {
            texture->SetHostImage(nullptr);
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
