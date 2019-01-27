#include <Luch/Render/Deferred/ResolveComputeRenderPass.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/Deferred/ResolveComputeContext.h>
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

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    const String ResolveComputeRenderPass::RenderPassName{"ResolveCompute"};

    ResolveComputeRenderPass::ResolveComputeRenderPass(
        ResolveComputePersistentContext* aPersistentContext,
        ResolveComputeTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        auto node = builder->AddComputeRenderPass(RenderPassName, this);

        for(int32 i = 0; i < transientContext->gbuffer.color.size(); i++)
        {
            gbuffer.color[i] = node->ReadsTexture(transientContext->gbuffer.color[i]);
        }

        gbuffer.depthStencil = node->ReadsTexture(transientContext->gbuffer.depthStencil);

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = ColorFormat;
        textureCreateInfo.width = transientContext->outputSize.width;
        textureCreateInfo.height = transientContext->outputSize.height;
        textureCreateInfo.usage = TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
        resolveTextureHandle = node->CreateTexture(textureCreateInfo);
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

    void ResolveComputeRenderPass::ExecuteComputeRenderPass(
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        for(int32 i = 0; i < gbuffer.color.size(); i++)
        {
            auto colorTexture = manager->GetTexture(gbuffer.color[i]);

            transientContext->gbufferTextureDescriptorSet->WriteTexture(
                persistentContext->colorTextureBindings[i],
                colorTexture);
        }

        auto depthStencilTexture = manager->GetTexture(gbuffer.depthStencil);

        transientContext->gbufferTextureDescriptorSet->WriteTexture(
            persistentContext->depthStencilTextureBinding,
            depthStencilTexture);

        auto luminanceTexture = manager->GetTexture(resolveTextureHandle);

        transientContext->luminanceTextureDescriptorSet->WriteTexture(
            persistentContext->luminanceTextureBinding,
            luminanceTexture);

        transientContext->gbufferTextureDescriptorSet->Update();

        cmdList->Begin();
        cmdList->BindPipelineState(persistentContext->pipelineState);

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
        cmdList->End();
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
            persistentContext->lightingParamsBinding,
            lightingParamsSuballocation.buffer,
            lightingParamsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->lightsBufferBinding,
            lightsSuballocation.buffer,
            lightsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->Update();
    }

    RefPtr<ComputePipelineState> ResolveComputeRenderPass::CreateResolvePipelineState(ResolveComputePersistentContext* context)
    {
        ComputePipelineStateCreateInfo ci;

        ci.name = "Resolve Compute";

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
        CameraResources* cameraResources)
    {
        auto context = MakeUnique<ResolveComputePersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;

        auto[kernelShaderLibraryCreated, createdKernelShaderLibrary] = RenderUtils::CreateShaderLibrary(
            device,
            "Data/Shaders/Deferred/",
            "resolve_compute",
            {});

        if (!kernelShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        auto [kernelShaderProgramCreateResult, kernelShaderProgram] = createdKernelShaderLibrary->CreateShaderProgram(
            ShaderStage::Compute,
            "kernel_main");

        if(kernelShaderProgramCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->kernelShader = std::move(kernelShaderProgram);

        context->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        context->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(2)
            .AddBinding(&context->lightingParamsBinding)
            .AddBinding(&context->lightsBufferBinding);

        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);

        for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
        {
            context->colorTextureBindings[i].OfType(ResourceType::Texture);
        }
        
        context->depthStencilTextureBinding.OfType(ResourceType::Texture);

        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
        gbufferTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(DeferredConstants::GBufferColorAttachmentCount + 1);

        for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
        {
            gbufferTextureDescriptorSetLayoutCreateInfo.AddBinding(&context->colorTextureBindings[i]);
        }

        gbufferTextureDescriptorSetLayoutCreateInfo.AddBinding(&context->depthStencilTextureBinding);

        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo luminanceTextureDescriptorSetLayoutCreateInfo;
        luminanceTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(1);

        luminanceTextureDescriptorSetLayoutCreateInfo.AddBinding(&context->luminanceTextureBinding);

        auto[createLuminanceTextureDescriptorSetLayoutResult, createdLuminanceTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(luminanceTextureDescriptorSetLayoutCreateInfo);
        if (createLuminanceTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->luminanceTextureDescriptorSetLayout = std::move(createdLuminanceTextureDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Compute, context->cameraResources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Compute, context->lightsBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Compute, context->gbufferTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Compute, context->luminanceTextureDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->pipelineLayout = std::move(createdPipelineLayout);

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

        auto [allocateGBufferTextureSetResult, allocatedGBufferTextureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->gbufferTextureDescriptorSetLayout);

        if (allocateGBufferTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->gbufferTextureDescriptorSet = std::move(allocatedGBufferTextureSet);

        auto [allocateLuminanceTextureSetResult, allocatedLuminanceTextureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->luminanceTextureDescriptorSetLayout);

        if (allocateLuminanceTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->luminanceTextureDescriptorSet = std::move(allocatedLuminanceTextureSet);

        auto [allocateLightsDescriptorSetResult, allocatedLightsBufferSet] = context->descriptorPool->AllocateDescriptorSet(
            persistentContext->lightsBufferDescriptorSetLayout);

        if(allocateLightsDescriptorSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->lightsBufferDescriptorSet = allocatedLightsBufferSet;

        return { true, std::move(context) };
    }
}
