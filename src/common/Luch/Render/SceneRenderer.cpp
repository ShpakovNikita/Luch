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
#include <Luch/Render/LightResources.h>
#include <Luch/Render/IBLRenderer.h>

#include <Luch/Render/Passes/DepthOnlyRenderPass.h>
#include <Luch/Render/Passes/DepthOnlyContext.h>

#include <Luch/Render/Passes/Forward/ForwardRenderPass.h>
#include <Luch/Render/Passes/Forward/ForwardContext.h>
#include <Luch/Render/Techniques/Forward/ForwardRendererContext.h>

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

        diffuseIlluminanceCubemapHandle = nullptr;
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

        cameraResources = PrepareCameraResources(context->device);
        if(cameraResources == nullptr)
        {
            return false;
        }

        indirectLightingResources = PrepareIndirectLightingResources(context->device);
        if(indirectLightingResources == nullptr)
        {
            return false;
        }

        lightResources = PrepareLightResources(context->device);
        if(lightResources == nullptr)
        {
            return false;
        }

        materialManager = MakeShared<MaterialManager>();

        auto materialManagerInitialized = materialManager->Initialize(context->device);

        if(!materialManagerInitialized)
        {
            return false;
        }

        iblRenderer = MakeUnique<IBLRenderer>(scene);

        bool iblRendererInitialized = iblRenderer->Initialize(
            context,
            materialManager,
            cameraResources,
            lightResources);

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
                materialManager->GetPersistentResources());

            if(!createDepthOnlyPersistentContextResult)
            {
                return false;
            }

            depthOnlyPersistentContext = std::move(createdDepthOnlyPersistentContext);
        }

        // Forward Persistent Context
        {
            ForwardPersistentContextCreateInfo createInfo;
            createInfo.device = context->device;
            createInfo.cameraResources = cameraResources.get();
            createInfo.materialResources = materialManager->GetPersistentResources();
            createInfo.lightResources = lightResources.get();
            createInfo.indirectLightingResources = indirectLightingResources.get();

            auto [createForwardPersistentContextResult, createdForwardPersistentContext] = ForwardRenderPass::PrepareForwardPersistentContext(
                createInfo);
            
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
                materialManager->GetPersistentResources(),
                indirectLightingResources.get(),
                lightResources.get());

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
            materialManager->GetPersistentResources());

        if(!createGBufferPersistentContextResult)
        {
            return false;
        }

        gbufferPersistentContext = std::move(createdGBufferPersistentContext);

        // Resolve (Compute) Persistent Context
        auto [createResolveComputePersistentContextResult, createdResolveComputePersistentContext] = ResolveComputeRenderPass::PrepareResolvePersistentContext(
            context->device,
            cameraResources.get(),
            indirectLightingResources.get(),
            lightResources.get());

        if(!createResolveComputePersistentContextResult)
        {
            return false;
        }

        resolveComputePersistentContext = std::move(createdResolveComputePersistentContext);

        // Resolve (Graphics) Persistent Context
        auto [createResolvePersistentContextResult, createdResolvePersistentContext] = ResolveRenderPass::PrepareResolvePersistentContext(
            context->device,
            cameraResources.get(),
            indirectLightingResources.get(),
            lightResources.get());

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

        auto& sceneProperties = scene->GetSceneProperties();

        auto lightProbeNodeIt = std::find_if(
            sceneProperties.lightProbeNodes.begin(),
            sceneProperties.lightProbeNodes.end(),
            [](const auto& node) { return node->GetLightProbe() != nullptr && node->GetLightProbe()->IsEnabled(); });

        if(lightProbeNodeIt == sceneProperties.lightProbeNodes.end())
        {
            return true;
        }

        const auto& lightProbeNode = *lightProbeNodeIt;
        const auto& lightProbe = lightProbeNode->GetLightProbe();

        IBLRequest iblRequest;
        iblRequest.position = lightProbeNode->GetWorldTransform() * Vec4{ 0, 0, 0, 1 };
        iblRequest.probeDiffuseIlluminance = lightProbe->HasDiffuseIlluminance();
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

        diffuseIlluminanceCubemap = probe.diffuseIlluminanceCubemap;
        specularReflectionCubemap = probe.specularReflectionCubemap;
        specularBRDFTexture = probe.specularBRDFTexture;

        return true;
    }

    void SceneRenderer::ResetIndirectLighting()
    {
        diffuseIlluminanceCubemap = nullptr;
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
            frame.diffuseIlluminanceCubemapHandle = frame.builder->GetResourceManager()->ImportTexture(diffuseIlluminanceCubemap);
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
            [this]()
            {
                resourcePool->Tick();
                semaphore->Signal();
            });

        frameIndex++;
    }

    bool SceneRenderer::PrepareForward(FrameResources& frame)
    {
        RenderMutableResource depthTextureHandle = config.useDepthPrepass ? frame.depthOnlyPass->GetDepthTextureHandle() : nullptr;

        ForwardTransientContextCreateInfo createInfo;
        createInfo.scene = scene;
        createInfo.outputSize = frame.outputSize;
        createInfo.sharedBuffer = frame.sharedBuffer;
        createInfo.descriptorPool = descriptorPool;
        createInfo.cameraBufferDescriptorSet = frame.cameraDescriptorSet;
        createInfo.useDepthPrepass = config.useDepthPrepass;

        if(config.useDiffuseGlobalIllumination)
        {
            createInfo.diffuseIlluminanceCubemapHandle = frame.diffuseIlluminanceCubemapHandle;
        }

        if(config.useSpecularGlobalIllumination)
        {
            createInfo.specularReflectionCubemapHandle = frame.specularReflectionCubemapHandle;
            createInfo.specularBRDFTextureHandle = frame.specularBRDFTextureHandle;
        }

        if(config.useDepthPrepass)
        {
            createInfo.depthStencilTextureHandle = frame.depthOnlyPass->GetDepthTextureHandle();
        }


        auto [result, transientContext] = ForwardRenderPass::PrepareForwardTransientContext(
            forwardPersistentContext.get(),
            createInfo);

        if(!result)
        {
            return false;
        }

        frame.forwardTransientContext = std::move(transientContext);

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
            transientContext->useDepthPrepass = config.useDepthPrepass;

            if(config.useDepthPrepass)
            {
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

            transientContext->outputSize = frame.outputSize;
            transientContext->scene = scene;
            transientContext->sharedBuffer = frame.sharedBuffer;
            transientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

            if(config.useDiffuseGlobalIllumination)
            {
                transientContext->diffuseIlluminanceCubemapHandle = frame.diffuseIlluminanceCubemapHandle;
            }

            if(config.useSpecularGlobalIllumination)
            {
                transientContext->specularReflectionCubemapHandle = frame.specularReflectionCubemapHandle;
                transientContext->specularBRDFTextureHandle = frame.specularBRDFTextureHandle;
            }

            transientContext->gbuffer = frame.gbufferPass->GetGBuffer();

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
                transientContext->diffuseIlluminanceCubemapHandle = frame.diffuseIlluminanceCubemapHandle;
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
        auto [result, tiledDeferredTransientContext] = TiledDeferredRenderPass::PrepareTiledDeferredTransientContext(
            tiledDeferredPersistentContext.get(),
            descriptorPool);

        if(!result)
        {
            LUCH_ASSERT(false);
            return false;
        }

        if(config.useDiffuseGlobalIllumination)
        {
            tiledDeferredTransientContext->diffuseIlluminanceCubemapHandle = frame.diffuseIlluminanceCubemapHandle;
        }

        if(config.useSpecularGlobalIllumination)
        {
            tiledDeferredTransientContext->specularReflectionCubemapHandle = frame.specularReflectionCubemapHandle;
            tiledDeferredTransientContext->specularBRDFTextureHandle = frame.specularBRDFTextureHandle;
        }

        tiledDeferredTransientContext->descriptorPool = descriptorPool;
        tiledDeferredTransientContext->outputSize = frame.outputSize;
        tiledDeferredTransientContext->scene = scene;
        tiledDeferredTransientContext->sharedBuffer = frame.sharedBuffer;
        tiledDeferredTransientContext->cameraBufferDescriptorSet = frame.cameraDescriptorSet;

        frame.tiledDeferredTransientContext = std::move(tiledDeferredTransientContext);

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

    UniquePtr<CameraPersistentResources> SceneRenderer::PrepareCameraResources(
        GraphicsDevice* device)
    {
        auto resources = MakeUnique<CameraPersistentResources>();

        {
            resources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);

            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .AddBinding(&resources->cameraUniformBufferBinding)
                .OfType(DescriptorSetType::Buffer);

            auto [result, descriptorSetLayout] = device->CreateDescriptorSetLayout(createInfo);
            if(result != GraphicsResult::Success)
            {
                return nullptr;
            }

            resources->cameraBufferDescriptorSetLayout = std::move(descriptorSetLayout);
        }

        {
            DescriptorPoolCreateInfo createInfo;
            createInfo.maxDescriptorSets = 10;
            createInfo.descriptorCount = 
            {
                { ResourceType::UniformBuffer, 10}
            };

            auto [result, descriptorPool] = device->CreateDescriptorPool(createInfo);
            if(result != GraphicsResult::Success)
            {
                return nullptr;
            }

            resources->descriptorPool = std::move(descriptorPool);
        }

        return resources;
    }

    UniquePtr<IndirectLightingPersistentResources> SceneRenderer::PrepareIndirectLightingResources(
        GraphicsDevice* device)
    {
        auto resources = MakeUnique<IndirectLightingPersistentResources>();

        {
            resources->diffuseIlluminanceCubemapBinding.OfType(ResourceType::Texture);
            resources->specularReflectionCubemapBinding.OfType(ResourceType::Texture);
            resources->specularBRDFTextureBinding.OfType(ResourceType::Texture);

            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(3)
                .AddBinding(&resources->diffuseIlluminanceCubemapBinding)
                .AddBinding(&resources->specularReflectionCubemapBinding)
                .AddBinding(&resources->specularBRDFTextureBinding);

            auto[result, descriptorSetLayout] = device->CreateDescriptorSetLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                return nullptr;
            }

            resources->indirectLightingTexturesDescriptorSetLayout = std::move(descriptorSetLayout);
        }

        return resources;
    }

    UniquePtr<LightPersistentResources> SceneRenderer::PrepareLightResources(GraphicsDevice* device)
    {
        auto resources = MakeUnique<LightPersistentResources>();

        resources->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        resources->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(2)
            .AddBinding(&resources->lightingParamsBinding)
            .AddBinding(&resources->lightsBufferBinding);

        auto [result, createdDescriptorSetLayout] = device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (result != GraphicsResult::Success)
        {
            return nullptr;
        }

        resources->lightsBufferDescriptorSetLayout = std::move(createdDescriptorSetLayout);

        return resources;
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
