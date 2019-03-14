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
#include <Luch/Render/MaterialManager.h>

#include <Luch/Render/Passes/IBL/EnvironmentCubemapRenderPass.h>
#include <Luch/Render/Passes/IBL/EnvironmentCubemapContext.h>

#include <Luch/Render/Passes/IBL/DiffuseIlluminanceRenderPass.h>
#include <Luch/Render/Passes/IBL/DiffuseIlluminanceContext.h>

#include <Luch/Render/Passes/IBL/SpecularReflectionRenderPass.h>
#include <Luch/Render/Passes/IBL/SpecularReflectionContext.h>

#include <Luch/Render/Passes/IBL/SpecularBRDFRenderPass.h>
#include <Luch/Render/Passes/IBL/SpecularBRDFContext.h>

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
        SharedPtr<MaterialManager> aMaterialManager,
        SharedPtr<CameraResources> aCameraResources)
    {
        context = aContext;
        cameraResources = aCameraResources;
        materialManager = aMaterialManager;

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
                cameraResources.get(),
                materialManager->GetResources());
            
            if(!result)
            {
                return false;
            }

            environmentCubemapPersistentContext = std::move(createdContext);
        }

        // Diffuse Illuminance Cubemap Persistent Context
        {
            auto [result, createdContext] = DiffuseIlluminanceRenderPass::PrepareDiffuseIlluminancePersistentContext(context->device);
            
            if(!result)
            {
                return false;
            }

            diffuseIlluminancePersistentContext = std::move(createdContext);
        }

        // Specular Reflection Persistent Context
        {
            auto [result, createdContext] = SpecularReflectionRenderPass::PrepareSpecularReflectionPersistentContext(context->device);

            if(!result)
            {
                return false;
            }

            specularReflectionPersistentContext = std::move(createdContext);
        }

        // Specular Reflection Persistent Context
        {
            auto [result, createdContext] = SpecularBRDFRenderPass::PrepareSpecularBRDFPersistentContext(context->device);

            if(!result)
            {
                return false;
            }

            specularBRDFPersistentContext = std::move(createdContext);
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
        diffuseIlluminancePersistentContext.reset();
        context.reset();
        cameraResources.reset();
        materialManager.reset();

        return true;
    }

    bool IBLRenderer::BeginRender(const IBLRequest& iblRequest)
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

        bool needEnvironmentMap = iblRequest.probeDiffuseIlluminance || iblRequest.probeSpecularReflection;

        if(needEnvironmentMap)
        {
            bool environmentMappingPrepared = PrepareEnvironmentMapping(iblRequest);
            if(!environmentMappingPrepared)
            {
                return false;
            }
        }

        if(iblRequest.probeDiffuseIlluminance)
        {
            bool diffuseIlluminancePrepared = PrepareDiffuseIlluminance(iblRequest);
            if(!diffuseIlluminancePrepared)
            {
                return false;
            }
        }

        if(iblRequest.probeSpecularReflection)
        {
            bool specularReflectionPrepared = PrepareSpecularReflection(iblRequest);
            if(!specularReflectionPrepared)
            {
                return false;
            }

        }

        if(iblRequest.computeSpecularBRDF)
        {
            bool specularBRDFPrepared = PrepareSpecularBRDF(iblRequest);
            if(!specularBRDFPrepared)
            {
                return false;
            }
        }

        return true;
    }

    bool IBLRenderer::PrepareScene()
    {
        for(auto& pass : environmentCubemapPasses)
        {
            if(pass != nullptr)
            {
                pass->PrepareScene();
            }
        }

        return true;
    }

    void IBLRenderer::UpdateScene()
    {
        auto& materials = scene->GetSceneProperties().materials;

        // TODO fix this
        for(auto& material : materials)
        {
            materialManager->UpdateMaterial(material, sharedBuffer.get());
        }

        // TODO fix this
        for(auto& pass : environmentCubemapPasses)
        {
            if(pass != nullptr)
            {
                pass->UpdateScene();
            }
        }
    }

    void IBLRenderer::ProbeIndirectLighting()
    {
        bool timedOut = probeReadySemaphore->Wait(0);
        LUCH_ASSERT(!timedOut);

        auto [buildResult, builtRenderGraph] = builder->Build();
        renderGraph = std::move(builtRenderGraph);

        auto commandLists = renderGraph->Execute();
        RenderUtils::SubmitCommandLists(context->commandQueue, commandLists, [this]()
        {
            probeReadySemaphore->Signal();
        });
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
        result.environmentCubemap = renderGraph->GetResourceManager()->ReleaseTexture(environmentLuminanceCubemapHandle);

        if(diffuseIlluminancePass != nullptr)
        {
            result.diffuseIlluminanceCubemap = renderGraph->GetResourceManager()->ReleaseTexture(diffuseIlluminancePass->GetIlluminanceCubemapHandle());
        }

        if(specularReflectionPass != nullptr)
        {
            result.specularReflectionCubemap = renderGraph->GetResourceManager()->ReleaseTexture(specularReflectionPass->GetSpecularReflectionCubemapHandle());
        }

        if(specularBRDFPass != nullptr)
        {
            result.specularBRDFTexture = renderGraph->GetResourceManager()->ReleaseTexture(specularBRDFPass->GetBRDFTextureHandle());
        }

        for(auto& pass : environmentCubemapPasses)
        {
            pass.reset();
        }

        diffuseIlluminancePass.release();
        specularReflectionTransientContext.reset();

        for(auto& transientContext : environmentCubemapTransientContexts)
        {
            transientContext.reset();
        }

        diffuseIlluminanceTransientContext.reset();
        specularReflectionTransientContext.reset();
        builder.reset();
        renderGraph.reset();

        renderSemaphore->Signal();

        return { true, result };
    }

    bool IBLRenderer::PrepareEnvironmentMapping(const IBLRequest& iblRequest)
    {
        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        TextureCreateInfo cubemapCreateInfo;
        cubemapCreateInfo.width = iblRequest.size.width;
        cubemapCreateInfo.height = iblRequest.size.height;
        cubemapCreateInfo.textureType = TextureType::TextureCube;
        cubemapCreateInfo.usage = TextureUsageFlags::ShaderRead;

        TextureCreateInfo luminanceCubemapCreateInfo = cubemapCreateInfo;
        luminanceCubemapCreateInfo.format = EnvironmentCubemapRenderPass::LuminanceFormat;
        luminanceCubemapCreateInfo.usage |= TextureUsageFlags::ColorAttachment;

        environmentLuminanceCubemapHandle = builder->GetResourceManager()->CreateTexture(luminanceCubemapCreateInfo);

        TextureCreateInfo depthCubemapCreateInfo = cubemapCreateInfo;
        depthCubemapCreateInfo.format = depthStencilFormat;
        depthCubemapCreateInfo.usage |= TextureUsageFlags::DepthStencilAttachment;

        environmentDepthCubemapHandle = builder->GetResourceManager()->CreateTexture(depthCubemapCreateInfo);

        for(int32 face = 0; face < 6; face++)
        {
            auto [result, transientContext] = EnvironmentCubemapRenderPass::PrepareEnvironmentCubemapTransientContext(
                environmentCubemapPersistentContext.get(),
                iblRequest.zNear,
                iblRequest.zFar,
                descriptorPool);

            if(!result)
            {
                return false;
            }

            transientContext->descriptorPool = descriptorPool;
            transientContext->outputSize = iblRequest.size;
            transientContext->scene = scene;
            transientContext->sharedBuffer = sharedBuffer;
            transientContext->environmentLuminanceCubemap = environmentLuminanceCubemapHandle;
            transientContext->environmentDepthCubemap = environmentDepthCubemapHandle;
            transientContext->faceIndex = face;
            transientContext->position = iblRequest.position;

            environmentCubemapTransientContexts[face] = std::move(transientContext);

            environmentCubemapPasses[face] = MakeUnique<EnvironmentCubemapRenderPass>(
                environmentCubemapPersistentContext.get(),
                environmentCubemapTransientContexts[face].get(),
                builder.get());

            // Do this to force render pass order
            environmentLuminanceCubemapHandle = environmentCubemapPasses[face]->GetEnvironmentLuminanceCubemapHandle();
            environmentDepthCubemapHandle = environmentCubemapPasses[face]->GetEnvironmentDepthCubemapHandle();
        }

        return true;
    }

    bool IBLRenderer::PrepareDiffuseIlluminance(const IBLRequest& iblRequest)
    {
        auto [result, transientContext] = DiffuseIlluminanceRenderPass::PrepareDiffuseIlluminanceTransientContext(
            diffuseIlluminancePersistentContext.get(),
            descriptorPool);

        if(!result)
        {
            return false;
        }

        transientContext->descriptorPool = descriptorPool;
        transientContext->outputSize = iblRequest.size;
        transientContext->scene = scene;
        transientContext->sharedBuffer = sharedBuffer;
        transientContext->luminanceCubemapHandle = environmentLuminanceCubemapHandle;

        diffuseIlluminanceTransientContext = std::move(transientContext);

        diffuseIlluminancePass = MakeUnique<DiffuseIlluminanceRenderPass>(
            diffuseIlluminancePersistentContext.get(),
            diffuseIlluminanceTransientContext.get(),
            builder.get());

        return true;
    }

    bool IBLRenderer::PrepareSpecularReflection(const IBLRequest& iblRequest)
    {
        auto [result, transientContext] = SpecularReflectionRenderPass::PrepareSpecularReflectionTransientContext(
            specularReflectionPersistentContext.get(),
            descriptorPool);

        if(!result)
        {
            return false;
        }

        transientContext->descriptorPool = descriptorPool;
        transientContext->outputSize = iblRequest.size;
        transientContext->sharedBuffer = sharedBuffer;
        transientContext->luminanceCubemapHandle = environmentLuminanceCubemapHandle;

        specularReflectionTransientContext = std::move(transientContext);

        specularReflectionPass = MakeUnique<SpecularReflectionRenderPass>(
            specularReflectionPersistentContext.get(),
            specularReflectionTransientContext.get(),
            builder.get());

        return true;
    }

    bool IBLRenderer::PrepareSpecularBRDF(const IBLRequest& iblRequest)
    {
        auto [result, transientContext] = SpecularBRDFRenderPass::PrepareSpecularBRDFTransientContext(
            specularBRDFPersistentContext.get(),
            descriptorPool);

        if(!result)
        {
            return false;
        }

        transientContext->descriptorPool = descriptorPool;
        transientContext->outputSize = { 128, 128 };
        transientContext->sharedBuffer = sharedBuffer;

        specularBRDFTransientContext = std::move(transientContext);

        specularBRDFPass = MakeUnique<SpecularBRDFRenderPass>(
            specularBRDFPersistentContext.get(),
            specularBRDFTransientContext.get(),
            builder.get());

        return true;
    }
}
