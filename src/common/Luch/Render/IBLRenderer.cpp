#include <Luch/Render/IBLRenderer.h>

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

#include <Luch/Render/Passes/IBL/EnvironmentCubemapRenderPass.h>
#include <Luch/Render/Passes/IBL/EnvironmentCubemapContext.h>

#include <Luch/Render/Passes/IBL/DiffuseIrradianceRenderPass.h>
#include <Luch/Render/Passes/IBL/DiffuseIrradianceContext.h>

#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphResourcePool.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Passes;
    using namespace IBL;
    using namespace Graph;

    IBLRenderer::IBLRenderer(
        RefPtr<SceneV1::Scene> aScene)
        : scene(std::move(aScene))
    {
    }

    IBLRenderer::~IBLRenderer() = default;

    bool IBLRenderer::Initialize(
        SharedPtr<RenderContext> aContext,
        CameraResources* aCameraResources,
        MaterialResources* aMaterialResources)
    {
        context = aContext;
        cameraResources = aCameraResources;
        materialResources = aMaterialResources;

        resourcePool = MakeUnique<RenderGraphResourcePool>(context->device);

        {
            auto [result, createdCommandPool] = context->commandQueue->CreateCommandPool();
            if(result != GraphicsResult::Success)
            {
                return false;
            }

            commandPool = std::move(createdCommandPool);
        }

        // Environment Cubemap Persistent Context
        {
            auto [result, createdContext] = EnvironmentCubemapRenderPass::PrepareEnvironmentCubemapPersistentContext(
                context->device,
                cameraResources,
                materialResources);
            
            if(!result)
            {
                return false;
            }

            environmentCubemapPersistentContext = std::move(createdContext);
        }

        // Diffuse Irradiance Cubemap Persistent Context
        {
            auto [result, createdContext] = DiffuseIrradianceRenderPass::PrepareDiffuseIrradiancePersistentContext(context->device);
            
            if(!result)
            {
                return false;
            }

            diffuseIrradiancePersistentContext = std::move(createdContext);
        }

        {
            DescriptorPoolCreateInfo descriptorPoolCreateInfo;
            descriptorPoolCreateInfo.maxDescriptorSets = DescriptorSetCount;
            descriptorPoolCreateInfo.descriptorCount =
            {
                { ResourceType::Texture, DescriptorCount },
                { ResourceType::Sampler, DescriptorCount },
                { ResourceType::UniformBuffer, DescriptorCount },
            };

            auto [result, createdDescriptorPool] = context->device->CreateDescriptorPool(descriptorPoolCreateInfo);
            if(result != GraphicsResult::Success)
            {
                return false;
            }

            descriptorPool = std::move(createdDescriptorPool);
        }

        {
            BufferCreateInfo sharedBufferCreateInfo;
            sharedBufferCreateInfo.length = SharedBufferSize;
            sharedBufferCreateInfo.storageMode = ResourceStorageMode::Shared;
            sharedBufferCreateInfo.usage = BufferUsageFlags::Uniform;

            auto [result, createdBuffer] = context->device->CreateBuffer(sharedBufferCreateInfo);
            if(result != GraphicsResult::Success)
            {
                return false;
            }

            sharedBuffer = MakeShared<SharedBuffer>(std::move(createdBuffer));
        }

        {
            auto [createSemaphoreResult, createdSemaphore] = context->device->CreateSemaphore(1);
            if(createSemaphoreResult != GraphicsResult::Success)
            {
                return false;
            }

            renderSemaphore = std::move(createdSemaphore);
        }

        {
            auto [createSemaphoreResult, createdSemaphore] = context->device->CreateSemaphore(1);
            if(createSemaphoreResult != GraphicsResult::Success)
            {
                return false;
            }

            probeReadySemaphore = std::move(createdSemaphore);
        }

        return true;
    }

    bool IBLRenderer::Deinitialize()
    {
        commandPool.Release();
        environmentCubemapPersistentContext.reset();
        diffuseIrradiancePersistentContext.reset();
        context.reset();
        cameraResources = nullptr;
        materialResources = nullptr;

        return true;
    }

    bool IBLRenderer::BeginRender()
    {
        bool timedOut = renderSemaphore->Wait();
        if(timedOut)
        {
            return false;
        }

        sharedBuffer->Reset();

        builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = builder->Initialize(context->device, commandPool, resourcePool.get());
        if(!builderInitialized)
        {
            return false;
        }

        bool environmentMappingPrepared = PrepareEnvironmentMapping();
        if(!environmentMappingPrepared)
        {
            return false;
        }

        bool diffuseIrradiancePrepared = PrepareDiffuseIrradiance();
        if(!diffuseIrradiancePrepared)
        {
            return false;
        }

        return true;
    }

    bool IBLRenderer::PrepareScene()
    {
        environmentCubemapPass->PrepareScene();

        return true;
    }

    void IBLRenderer::UpdateScene()
    {
        environmentCubemapPass->UpdateScene();
    }

    void IBLRenderer::ProbeIndirectLighting(Vec3 position)
    {
        environmentCubemapTransientContext->position = position;

        bool timedOut = probeReadySemaphore->Wait(0);
        LUCH_ASSERT(!timedOut);

        auto [buildResult, builtRenderGraph] = builder->Build();
        renderGraph = std::move(builtRenderGraph);

        auto commandLists = renderGraph->Execute();
        for(auto& commandList : commandLists)
        {
            context->commandQueue->Submit(commandList, [this]()
            {
                probeReadySemaphore->Signal();
            });
        }
    }

    ResultValue<bool, IBLResult> IBLRenderer::EndRender()
    {
        bool timedOut = probeReadySemaphore->Wait();

        if(timedOut)
        {
            return { false };
        }

        probeReadySemaphore->Signal();

        IBLResult result;
        result.environmentCubemap = renderGraph->GetResourceManager()->ReleaseTexture(environmentCubemapPass->GetEnvironmentLuminanceCubemapHandle());
        result.diffuseIrradianceCubemap = renderGraph->GetResourceManager()->ReleaseTexture(diffuseIrradiancePass->GetIrradianceCubemapHandle());

        environmentCubemapPass.reset();
        diffuseIrradiancePass.release();
        environmentCubemapTransientContext.reset();
        diffuseIrradianceTransientContext.reset();
        builder.reset();
        renderGraph.reset();

        return { true, result };
    }

    bool IBLRenderer::PrepareEnvironmentMapping()
    {
        auto [result, transientContext] = EnvironmentCubemapRenderPass::PrepareEnvironmentCubemapTransientContext(
            environmentCubemapPersistentContext.get(),
            descriptorPool);

        if(!result)
        {
            return false;
        }

        environmentCubemapTransientContext = std::move(transientContext);

        environmentCubemapTransientContext->descriptorPool = descriptorPool;
        environmentCubemapTransientContext->outputSize = { 256, 256 };
        environmentCubemapTransientContext->scene = scene;
        environmentCubemapTransientContext->sharedBuffer = sharedBuffer;
        environmentCubemapPass = MakeUnique<EnvironmentCubemapRenderPass>(
            environmentCubemapPersistentContext.get(),
            environmentCubemapTransientContext.get(),
            builder.get());

        return true;
    }

    bool IBLRenderer::PrepareDiffuseIrradiance()
    {
        auto [result, transientContext] = DiffuseIrradianceRenderPass::PrepareDiffuseIrradianceTransientContext(
            diffuseIrradiancePersistentContext.get(),
            descriptorPool);

        if(!result)
        {
            return false;
        }

        diffuseIrradianceTransientContext = std::move(transientContext);

        diffuseIrradianceTransientContext->descriptorPool = descriptorPool;
        diffuseIrradianceTransientContext->outputSize = { 128, 128 };
        diffuseIrradianceTransientContext->scene = scene;
        diffuseIrradianceTransientContext->sharedBuffer = sharedBuffer;
        diffuseIrradianceTransientContext->luminanceCubemapHandle = environmentCubemapPass->GetEnvironmentLuminanceCubemapHandle();
        diffuseIrradiancePass = MakeUnique<DiffuseIrradianceRenderPass>(
            diffuseIrradiancePersistentContext.get(),
            diffuseIrradianceTransientContext.get(),
            builder.get());

        return true;
    }
}
