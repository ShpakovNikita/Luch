#include <Luch/Render/Passes/IBL/DiffuseIlluminanceRenderPass.h>
#include <Luch/Render/Passes/IBL/DiffuseIlluminanceContext.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/CameraResources.h>
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

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;

    const String DiffuseIlluminanceRenderPass::RenderPassName{"DiffuseIlluminance"};

    DiffuseIlluminanceRenderPass::DiffuseIlluminanceRenderPass(
        DiffuseIlluminancePersistentContext* aPersistentContext,
        DiffuseIlluminanceTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
    }

    DiffuseIlluminanceRenderPass::~DiffuseIlluminanceRenderPass() = default;

    void DiffuseIlluminanceRenderPass::Initialize(RenderGraphBuilder* builder)
    {
        auto node = builder->AddComputePass(RenderPassName, this);

        luminanceCubemapHandle = node->ReadsTexture(transientContext->luminanceCubemapHandle);

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = IlluminanceFormat;
        textureCreateInfo.textureType = TextureType::TextureCube;
        textureCreateInfo.width = transientContext->outputSize.width;
        textureCreateInfo.height = transientContext->outputSize.height;
        textureCreateInfo.usage = TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
        illuminanceCubemapHandle = node->CreateTexture(textureCreateInfo);
    }

    void DiffuseIlluminanceRenderPass::ExecuteComputePass(
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        auto luminanceCubemap = manager->GetTexture(luminanceCubemapHandle);

        transientContext->cubemapDescriptorSet->WriteTexture(
            persistentContext->luminanceCubemapBinding,
            luminanceCubemap);

        auto illuminanceCubemap = manager->GetTexture(illuminanceCubemapHandle);

        transientContext->cubemapDescriptorSet->WriteTexture(
            persistentContext->illuminanceCubemapBinding,
            illuminanceCubemap);

        transientContext->cubemapDescriptorSet->Update();

        cmdList->BindPipelineState(persistentContext->pipelineState);

        cmdList->BindTextureDescriptorSet(
            persistentContext->pipelineLayout,
            transientContext->cubemapDescriptorSet);

        int32 threadgroupRows = (transientContext->outputSize.height + ThreadsPerThreadgroup.height - 1) / ThreadsPerThreadgroup.height;
        int32 threadgroupColumns = (transientContext->outputSize.width + ThreadsPerThreadgroup.width - 1) / ThreadsPerThreadgroup.width;

        cmdList->DispatchThreadgroups({threadgroupColumns, threadgroupRows, 6}, ThreadsPerThreadgroup);
    }

    RefPtr<ComputePipelineState> DiffuseIlluminanceRenderPass::CreateDiffuseIlluminancePipelineState(DiffuseIlluminancePersistentContext* context)
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

    ResultValue<bool, UniquePtr<DiffuseIlluminancePersistentContext>> DiffuseIlluminanceRenderPass::PrepareDiffuseIlluminancePersistentContext(GraphicsDevice* device)
    {
        auto context = MakeUnique<DiffuseIlluminancePersistentContext>();
        context->device = device;

        {
            auto[kernelShaderLibraryCreated, createdKernelShaderLibrary] = RenderUtils::CreateShaderLibrary(
                device,
                "Data/Shaders/",
                "Data/Shaders/IBL/",
                "diffuse_illuminance_compute",
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
        }

        {
            context->luminanceCubemapBinding.OfType(ResourceType::Texture);
            context->illuminanceCubemapBinding.OfType(ResourceType::Texture);

            DescriptorSetLayoutCreateInfo cubemapDescriptorSetLayoutCreateInfo;
            cubemapDescriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(2)
                .AddBinding(&context->luminanceCubemapBinding)
                .AddBinding(&context->illuminanceCubemapBinding);

            auto[createCubemapDescriptorSetLayoutResult, createdCubemapDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cubemapDescriptorSetLayoutCreateInfo);
            if (createCubemapDescriptorSetLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->cubemapDescriptorSetLayout = std::move(createdCubemapDescriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo
                .AddSetLayout(ShaderStage::Compute, context->cubemapDescriptorSetLayout);

            auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
            if (createPipelineLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->pipelineLayout = std::move(createdPipelineLayout);
        }

        // TODO result
        context->pipelineState = CreateDiffuseIlluminancePipelineState(context.get());

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<DiffuseIlluminanceTransientContext>> DiffuseIlluminanceRenderPass::PrepareDiffuseIlluminanceTransientContext(
        DiffuseIlluminancePersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<DiffuseIlluminanceTransientContext>();
        context->descriptorPool = descriptorPool;

        {
            auto [allocateCubemapDescriptorSetResult, allocatedCubemapDescriptorSet] = context->descriptorPool->AllocateDescriptorSet(
                    persistentContext->cubemapDescriptorSetLayout);

            if (allocateCubemapDescriptorSetResult != GraphicsResult::Success)
            {
                return { false };
            }

            context->cubemapDescriptorSet = std::move(allocatedCubemapDescriptorSet);
        }

        return { true, std::move(context) };
    }
}
