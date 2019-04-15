#include <Luch/Render/Passes/IBL/SpecularBRDFRenderPass.h>
#include <Luch/Render/Passes/IBL/SpecularBRDFContext.h>
#include <Luch/Render/Passes/IBL/IBLCommon.h>
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

    const String SpecularBRDFRenderPass::RenderPassName{"SpecularBRDF"};

    SpecularBRDFRenderPass::SpecularBRDFRenderPass(
        SpecularBRDFPersistentContext* aPersistentContext,
        SpecularBRDFTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
    }

    SpecularBRDFRenderPass::~SpecularBRDFRenderPass() = default;

    void SpecularBRDFRenderPass::Initialize(RenderGraphBuilder* builder)
    {
        auto node = builder->AddComputePass(RenderPassName, this);

        {
            TextureCreateInfo textureCreateInfo;
            textureCreateInfo.format = BRDFFormat;
            textureCreateInfo.textureType = TextureType::Texture2D;
            textureCreateInfo.width = transientContext->outputSize.width;
            textureCreateInfo.height = transientContext->outputSize.height;
            textureCreateInfo.usage = TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
            brdfTextureHandle = node->CreateTexture(textureCreateInfo);
        }
    }

    void SpecularBRDFRenderPass::ExecuteComputePass(
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        ComputeBRDF(manager, cmdList);
    }

    void SpecularBRDFRenderPass::ComputeBRDF(
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        auto brdfTexture = manager->GetTexture(brdfTextureHandle);

        transientContext->brdfDescriptorSet->WriteTexture(
            persistentContext->brdfTextureBinding,
            brdfTexture);

        transientContext->brdfDescriptorSet->Update();

        cmdList->BindPipelineState(persistentContext->brdfPipelineState);

        cmdList->BindTextureDescriptorSet(
            persistentContext->brdfPipelineLayout,
            transientContext->brdfDescriptorSet);

        int32 threadgroupRows = (transientContext->outputSize.height + ThreadsPerThreadgroup.height - 1) / ThreadsPerThreadgroup.height;
        int32 threadgroupColumns = (transientContext->outputSize.width + ThreadsPerThreadgroup.width - 1) / ThreadsPerThreadgroup.width;

        cmdList->DispatchThreadgroups({threadgroupColumns, threadgroupRows, 1}, ThreadsPerThreadgroup);
    }

    RefPtr<ComputePipelineState> SpecularBRDFRenderPass::CreateBRDFPipelineState(SpecularBRDFPersistentContext* context)
    {
        ComputePipelineStateCreateInfo ci;

        ci.name = RenderPassName;

        ci.pipelineLayout = context->brdfPipelineLayout;
        ci.kernelProgram = context->brdfKernelShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreateComputePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<SpecularBRDFPersistentContext>> SpecularBRDFRenderPass::PrepareSpecularBRDFPersistentContext(GraphicsDevice* device)
    {
        auto context = MakeUnique<SpecularBRDFPersistentContext>();
        context->device = device;

        {
            auto[kernelShaderLibraryCreated, createdKernelShaderLibrary] = RenderUtils::CreateShaderLibrary(
                device,
                "Data/Shaders/",
                "Data/Shaders/IBL/",
                "specular_brdf_compute",
                {});

            if (!kernelShaderLibraryCreated)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            {
                auto [kernelShaderProgramCreateResult, kernelShaderProgram] = createdKernelShaderLibrary->CreateShaderProgram(
                    ShaderStage::Compute,
                    "brdf_kernel");

                if(kernelShaderProgramCreateResult != GraphicsResult::Success)
                {
                    LUCH_ASSERT(false);
                    return { false };
                }

                context->brdfKernelShader = std::move(kernelShaderProgram);
            }
        }

        {
            context->brdfTextureBinding.OfType(ResourceType::Texture);

            DescriptorSetLayoutCreateInfo cubemapDescriptorSetLayoutCreateInfo;
            cubemapDescriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(1)
                .AddBinding(&context->brdfTextureBinding);

            auto[result, createdDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cubemapDescriptorSetLayoutCreateInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->brdfDescriptorSetLayout = std::move(createdDescriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo
                .AddSetLayout(ShaderStage::Compute, context->brdfDescriptorSetLayout);

            auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
            if (createPipelineLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->brdfPipelineLayout = std::move(createdPipelineLayout);
        }

        context->brdfPipelineState = CreateBRDFPipelineState(context.get());

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<SpecularBRDFTransientContext>> SpecularBRDFRenderPass::PrepareSpecularBRDFTransientContext(
        SpecularBRDFPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<SpecularBRDFTransientContext>();
        context->descriptorPool = descriptorPool;

        {
            auto [result, allocatedDescriptorSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->brdfDescriptorSetLayout);

            if (result != GraphicsResult::Success)
            {
                return { false };
            }

            context->brdfDescriptorSet = std::move(allocatedDescriptorSet);
        }

        return { true, std::move(context) };
    }
}
