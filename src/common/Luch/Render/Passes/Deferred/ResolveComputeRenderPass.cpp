#include <Luch/Render/Passes/Deferred/ResolveComputeRenderPass.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/IndirectLightingResources.h>
#include <Luch/Render/LightResources.h>
#include <Luch/Render/Passes/Deferred/ResolveComputeContext.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/Light.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/ComputeCommandList.h>
#include <Luch/Graphics/Texture.h>
#include <Luch/Graphics/ComputePipelineState.h>
#include <Luch/Graphics/ComputePipelineStateCreateInfo.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>

namespace Luch::Render::Passes::Deferred
{
    using namespace Graphics;

    const String ResolveComputeRenderPass::RenderPassName{"ResolveCompute"};

    ResolveComputeRenderPass::ResolveComputeRenderPass(
        ResolveComputePersistentContext* aPersistentContext,
        ResolveComputeTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
    }

    ResolveComputeRenderPass::~ResolveComputeRenderPass() = default;

    void ResolveComputeRenderPass::PrepareScene()
    {
    }

    void ResolveComputeRenderPass::UpdateScene()
    {
        const auto& sceneProperties = transientContext->scene->GetSceneProperties();

        RefPtrVector<SceneV1::Node> lightNodes(sceneProperties.lightNodes.begin(), sceneProperties.lightNodes.end());

        UpdateLights(lightNodes);
    }

    void ResolveComputeRenderPass::Initialize(RenderGraphBuilder* builder)
    {
        auto node = builder->AddComputePass(RenderPassName, this);

        for(uint32 i = 0; i < transientContext->gbuffer.color.size(); i++)
        {
            gbuffer.color[i] = node->ReadsTexture(transientContext->gbuffer.color[i]);
        }

        gbuffer.depthStencil = node->ReadsTexture(transientContext->gbuffer.depthStencil);

        if(transientContext->diffuseIlluminanceCubemapHandle)
        {
            diffuseIlluminanceCubemapHandle = node->ReadsTexture(transientContext->diffuseIlluminanceCubemapHandle);
        }

        if(transientContext->specularReflectionCubemapHandle && transientContext->specularBRDFTextureHandle)
        {
            specularReflectionCubemapHandle = node->ReadsTexture(transientContext->specularReflectionCubemapHandle);
            specularBRDFTextureHandle = node->ReadsTexture(transientContext->specularBRDFTextureHandle);
        }

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = ColorFormat;
        textureCreateInfo.width = transientContext->outputSize.width;
        textureCreateInfo.height = transientContext->outputSize.height;
        textureCreateInfo.usage = TextureUsageFlags::ColorAttachment | TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
        luminanceTextureHandle = node->CreateTexture(textureCreateInfo);
    }

    void ResolveComputeRenderPass::ExecuteComputePass(
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        UpdateGBufferDescriptorSet(manager, transientContext->gbufferTextureDescriptorSet);
        UpdateIndirectLightingDescriptorSet(manager, transientContext->indirectLightingTexturesDescriptorSet);

        auto luminanceTexture = manager->GetTexture(luminanceTextureHandle);

        transientContext->luminanceTextureDescriptorSet->WriteTexture(
            persistentContext->luminanceTextureBinding,
            luminanceTexture);
        
        transientContext->luminanceTextureDescriptorSet->Update();

        cmdList->BindPipelineState(persistentContext->pipelineState);

        cmdList->BindTextureDescriptorSet(
            persistentContext->pipelineLayout,
            transientContext->indirectLightingTexturesDescriptorSet);

        cmdList->BindTextureDescriptorSet(
            persistentContext->pipelineLayout,
            transientContext->gbufferTextureDescriptorSet);

        cmdList->BindTextureDescriptorSet(
            persistentContext->pipelineLayout,
            transientContext->luminanceTextureDescriptorSet);

        cmdList->BindBufferDescriptorSet(
            persistentContext->pipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        cmdList->BindBufferDescriptorSet(
            persistentContext->pipelineLayout,
            transientContext->lightsBufferDescriptorSet);

        int32 threadgroupRows = (transientContext->outputSize.height + ThreadsPerThreadgroup.height - 1) / ThreadsPerThreadgroup.height;
        int32 threadgroupColumns = (transientContext->outputSize.width + ThreadsPerThreadgroup.width - 1) / ThreadsPerThreadgroup.width;
        cmdList->DispatchThreadgroups({threadgroupColumns, threadgroupRows, 1}, ThreadsPerThreadgroup);
    }

    void ResolveComputeRenderPass::UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes)
    {
        Vector<LightUniform> lightUniforms;

        for(const auto& lightNode : lightNodes)
        {
            const auto& light = lightNode->GetLight();
            LUCH_ASSERT(light != nullptr);

            if(light->IsEnabled())
            {
                LightUniform lightUniform = RenderUtils::GetLightUniform(light, lightNode->GetWorldTransform());
                lightUniforms.push_back(lightUniform);
            }
        }

        int32 enabledLightsCount = lightNodes.size();

        LightingParamsUniform lightingParams;
        lightingParams.lightCount = enabledLightsCount;

        auto lightingParamsSuballocation = transientContext->sharedBuffer->Suballocate(sizeof(LightingParamsUniform), 256);
        auto lightsSuballocation = transientContext->sharedBuffer->Suballocate(enabledLightsCount * sizeof(LightUniform), 256);

        memcpy(lightingParamsSuballocation.offsetMemory, &lightingParams, sizeof(LightingParamsUniform));
        memcpy(lightsSuballocation.offsetMemory, lightUniforms.data(), enabledLightsCount * sizeof(LightUniform));

        transientContext->lightsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->lightResources->lightingParamsBinding,
            lightingParamsSuballocation.buffer,
            lightingParamsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->lightResources->lightsBufferBinding,
            lightsSuballocation.buffer,
            lightsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->Update();
    }

    void ResolveComputeRenderPass::UpdateGBufferDescriptorSet(
        RenderGraphResourceManager* manager,
        DescriptorSet* descriptorSet)
    {
        for(uint32 i = 0; i < gbuffer.color.size(); i++)
        {
            auto colorTexture = manager->GetTexture(gbuffer.color[i]);

            descriptorSet->WriteTexture(
                persistentContext->colorTextureBindings[i],
                colorTexture);
        }

        auto depthStencilTexture = manager->GetTexture(gbuffer.depthStencil);

        descriptorSet->WriteTexture(
            persistentContext->depthStencilTextureBinding,
            depthStencilTexture);

        descriptorSet->Update();
    }

    void ResolveComputeRenderPass::UpdateIndirectLightingDescriptorSet(
        RenderGraphResourceManager* manager,
        DescriptorSet* descriptorSet)
    {
        if(diffuseIlluminanceCubemapHandle)
        {
            auto diffuseIlluminanceCubemap = manager->GetTexture(diffuseIlluminanceCubemapHandle);

            transientContext->indirectLightingTexturesDescriptorSet->WriteTexture(
                persistentContext->indirectLightingResources->diffuseIlluminanceCubemapBinding,
                diffuseIlluminanceCubemap);
        }

        if(specularReflectionCubemapHandle && specularBRDFTextureHandle)
        {
            auto specularReflectionCubemap = manager->GetTexture(specularReflectionCubemapHandle);

            descriptorSet->WriteTexture(
                persistentContext->indirectLightingResources->specularReflectionCubemapBinding,
                specularReflectionCubemap);

            auto specularBRDFTexture = manager->GetTexture(specularBRDFTextureHandle);

            descriptorSet->WriteTexture(
                persistentContext->indirectLightingResources->specularBRDFTextureBinding,
                specularBRDFTexture);
        }

        descriptorSet->Update();
    }

    RefPtr<ComputePipelineState> ResolveComputeRenderPass::CreateResolvePipelineState(ResolveComputePersistentContext* context)
    {
        ComputePipelineStateCreateInfo ci;

        ci.name = RenderPassName;

        ci.pipelineLayout = context->pipelineLayout;
        ci.kernelProgram = context->kernelShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreateComputePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<ResolveComputePersistentContext>> ResolveComputeRenderPass::PrepareResolvePersistentContext(
        GraphicsDevice* device,
        CameraPersistentResources* cameraResources,
        IndirectLightingPersistentResources* indirectLightingResources,
        LightPersistentResources* lightResources)
    {
        auto context = MakeUnique<ResolveComputePersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;
        context->indirectLightingResources = indirectLightingResources;
        context->lightResources = lightResources;

        {
            auto [libraryCreated, createdKernelShaderLibrary] = RenderUtils::CreateShaderLibrary(
                device,
                "Data/Shaders/",
                "Data/Shaders/Deferred/",
                "resolve_compute",
                {});

            if (!libraryCreated)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            auto [result, kernelShaderProgram] = createdKernelShaderLibrary->CreateShaderProgram(
                ShaderStage::Compute,
                "kernel_main");

            if(result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->kernelShader = std::move(kernelShaderProgram);
        }

        {
            for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
            {
                context->colorTextureBindings[i].OfType(ResourceType::Texture);
            }
            
            context->depthStencilTextureBinding.OfType(ResourceType::Texture);

            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(DeferredConstants::GBufferColorAttachmentCount + 1);

            for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
            {
                createInfo.AddBinding(&context->colorTextureBindings[i]);
            }

            createInfo.AddBinding(&context->depthStencilTextureBinding);

            auto [result, descriptorSetLayout] = context->device->CreateDescriptorSetLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->gbufferTextureDescriptorSetLayout = std::move(descriptorSetLayout);
        }

        {
            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(1);

            createInfo.AddBinding(&context->luminanceTextureBinding);

            auto[result, descriptorSetLayout] = context->device->CreateDescriptorSetLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->luminanceTextureDescriptorSetLayout = std::move(descriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo createInfo;
            createInfo
                .AddSetLayout(ShaderStage::Compute, context->cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Compute, context->lightResources->lightsBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Compute, context->gbufferTextureDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Compute, context->luminanceTextureDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Compute, context->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            auto[result, pipelineLayout] = context->device->CreatePipelineLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->pipelineLayout = std::move(pipelineLayout);
        }

        // TODO result
        context->pipelineState = CreateResolvePipelineState(context.get());

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<ResolveComputeTransientContext>> ResolveComputeRenderPass::PrepareResolveTransientContext(
        ResolveComputePersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<ResolveComputeTransientContext>();
        context->descriptorPool = descriptorPool;

        {
            auto [result, textureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->gbufferTextureDescriptorSetLayout);

            if (result != GraphicsResult::Success)
            {
                return { false };
            }

            context->gbufferTextureDescriptorSet = std::move(textureSet);
        }

        {
            auto [result, textureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->luminanceTextureDescriptorSetLayout);

            if (result != GraphicsResult::Success)
            {
                return { false };
            }

            context->luminanceTextureDescriptorSet = std::move(textureSet);
        }

        {
            auto [result, bufferSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->lightResources->lightsBufferDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->lightsBufferDescriptorSet = std::move(bufferSet);
        }

        {
            auto [result, textureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->indirectLightingTexturesDescriptorSet = std::move(textureSet);
        }

        return { true, std::move(context) };
    }
}
