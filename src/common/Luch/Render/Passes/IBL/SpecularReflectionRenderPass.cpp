#include <Luch/Render/Passes/IBL/SpecularReflectionRenderPass.h>
#include <Luch/Render/Passes/IBL/SpecularReflectionContext.h>
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
#include <Luch/Graphics/CopyCommandList.h>
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

    const String SpecularReflectionRenderPass::RenderPassName{"SpecularReflection"};

    SpecularReflectionRenderPass::SpecularReflectionRenderPass(
        SpecularReflectionPersistentContext* aPersistentContext,
        SpecularReflectionTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        // In metal on macOS you can't write to texture lods from shaders, 
        // so we will create separate textures and copy them into final one using
        // copy pass
        auto computeNode = builder->AddComputePass(RenderPassName, this);
        auto copyNode = builder->AddCopyPass(RenderPassName, this);

        luminanceCubemapHandle = computeNode->ReadsTexture(transientContext->luminanceCubemapHandle);
        LUCH_ASSERT(transientContext->outputSize.width == transientContext->outputSize.height);

        mipmapLevelCount = static_cast<int32>(floor(log2f(transientContext->outputSize.width))) + 1;
        specularReflectionCubemapHandles.resize(mipmapLevelCount);

        for(int32 i = 0; i < mipmapLevelCount; i++)
        {
            TextureCreateInfo textureCreateInfo;
            textureCreateInfo.format = SpecularReflectionFormat;
            textureCreateInfo.textureType = TextureType::TextureCube;
            textureCreateInfo.width = transientContext->outputSize.width >> i;
            textureCreateInfo.height = transientContext->outputSize.height >> i;
            textureCreateInfo.usage = TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
            auto mipLevelCubemapHandle = computeNode->CreateTexture(textureCreateInfo);
            specularReflectionCubemapHandles[i] = mipLevelCubemapHandle;
            copyNode->ReadsTexture(mipLevelCubemapHandle);
        }

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = SpecularReflectionFormat;
        textureCreateInfo.textureType = TextureType::TextureCube;
        textureCreateInfo.width = transientContext->outputSize.width;
        textureCreateInfo.height = transientContext->outputSize.height;
        textureCreateInfo.usage = TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
        textureCreateInfo.mipmapLevelCount = mipmapLevelCount;
        finalSpecularReflectionCubemapHandle = copyNode->CreateTexture(textureCreateInfo);
    }

    SpecularReflectionRenderPass::~SpecularReflectionRenderPass() = default;

    void SpecularReflectionRenderPass::ExecuteComputePass(
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        for(int32 i = 0; i < mipmapLevelCount; i++)
        {
            ComputeSpecularReflection(i, manager, cmdList);
        }
    }

    void SpecularReflectionRenderPass::ExecuteCopyPass(
        RenderGraphResourceManager* manager,
        CopyCommandList* cmdList)
    {
        auto finalTexture = manager->GetTexture(finalSpecularReflectionCubemapHandle);
        for(int32 i = 0; i < mipmapLevelCount; i++)
        {
            auto mipTexture = manager->GetTexture(specularReflectionCubemapHandles[i]);
            for(int32 face = 0; face < 6; face++)
            {
                TextureToTextureCopy copy;
                copy.sourceLevel = 0;
                copy.sourceSize = { transientContext->outputSize.width >> i, transientContext->outputSize.height >> i };
                copy.sourceSlice = face;
                copy.destinationLevel = i;
                copy.destinationSlice = face;

                cmdList->CopyTextureToTexture(mipTexture, finalTexture, copy);
            }
        }
    }

    void SpecularReflectionRenderPass::ComputeSpecularReflection(
        int32 mipLevel,
        RenderGraphResourceManager* manager,
        ComputeCommandList* cmdList)
    {
        SpecularReflectionParamsUniform params;
        params.mipLevel = static_cast<uint16>(mipLevel);
        params.mipLevel = static_cast<uint16>(mipmapLevelCount);

        auto paramsSuballocation = transientContext->sharedBuffer->Suballocate(sizeof(SpecularReflectionParamsUniform), 256);

        transientContext->paramsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->specularReflectionParamsBufferBinding,
            paramsSuballocation.buffer,
            paramsSuballocation.offset);
        
        transientContext->paramsBufferDescriptorSet->Update();

        auto luminanceCubemap = manager->GetTexture(luminanceCubemapHandle);

        transientContext->cubemapDescriptorSet->WriteTexture(
            persistentContext->luminanceCubemapBinding,
            luminanceCubemap);

        auto specularReflectionCubemap = manager->GetTexture(specularReflectionCubemapHandles[mipLevel]);

        transientContext->cubemapDescriptorSet->WriteTexture(
            persistentContext->specularReflectionCubemapBinding,
            specularReflectionCubemap);

        transientContext->cubemapDescriptorSet->Update();

        cmdList->BindPipelineState(persistentContext->specularReflectionPipelineState);

        cmdList->BindBufferDescriptorSet(
            persistentContext->specularReflectionPipelineLayout,
            transientContext->paramsBufferDescriptorSet);

        cmdList->BindTextureDescriptorSet(
            persistentContext->specularReflectionPipelineLayout,
            transientContext->cubemapDescriptorSet);

        int32 mipLevelWidth = transientContext->outputSize.width >> mipLevel;
        int32 mipLevelHeight = transientContext->outputSize.height >> mipLevel;

        int32 threadgroupRows = (mipLevelHeight + ThreadsPerThreadgroup.height - 1) / ThreadsPerThreadgroup.height;
        int32 threadgroupColumns = (mipLevelWidth + ThreadsPerThreadgroup.width - 1) / ThreadsPerThreadgroup.width;

        cmdList->DispatchThreadgroups({threadgroupColumns, threadgroupRows, 6}, ThreadsPerThreadgroup);
    }

    RefPtr<ComputePipelineState> SpecularReflectionRenderPass::CreateSpecularReflectionPipelineState(SpecularReflectionPersistentContext* context)
    {
        ComputePipelineStateCreateInfo ci;

        ci.name = RenderPassName;

        ci.pipelineLayout = context->specularReflectionPipelineLayout;
        ci.kernelProgram = context->specularReflectionKernelShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreateComputePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<SpecularReflectionPersistentContext>> SpecularReflectionRenderPass::PrepareSpecularReflectionPersistentContext(GraphicsDevice* device)
    {
        auto context = MakeUnique<SpecularReflectionPersistentContext>();
        context->device = device;

        {
            auto[kernelShaderLibraryCreated, createdKernelShaderLibrary] = RenderUtils::CreateShaderLibrary(
                device,
                "Data/Shaders/",
                "Data/Shaders/IBL/",
                "specular_reflection_compute",
                {});

            if (!kernelShaderLibraryCreated)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            {
                auto [kernelShaderProgramCreateResult, kernelShaderProgram] = createdKernelShaderLibrary->CreateShaderProgram(
                    ShaderStage::Compute,
                    "specular_reflection_kernel");

                if(kernelShaderProgramCreateResult != GraphicsResult::Success)
                {
                    LUCH_ASSERT(false);
                    return { false };
                }

                context->specularReflectionKernelShader = std::move(kernelShaderProgram);
            }
        }

        {
            context->luminanceCubemapBinding.OfType(ResourceType::Texture);
            context->specularReflectionCubemapBinding.OfType(ResourceType::Texture);

            DescriptorSetLayoutCreateInfo cubemapDescriptorSetLayoutCreateInfo;
            cubemapDescriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(2)
                .AddBinding(&context->luminanceCubemapBinding)
                .AddBinding(&context->specularReflectionCubemapBinding);

            auto[result, createdDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cubemapDescriptorSetLayoutCreateInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->cubemapDescriptorSetLayout = std::move(createdDescriptorSetLayout);
        }

        {
            context->specularReflectionParamsBufferBinding.OfType(ResourceType::UniformBuffer);

            DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
            descriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Buffer)
                .WithNBindings(1)
                .AddBinding(&context->specularReflectionParamsBufferBinding);

            auto[result, createdDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->paramsBufferDescriptorSetLayout = std::move(createdDescriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo
                .AddSetLayout(ShaderStage::Compute, context->cubemapDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Compute, context->paramsBufferDescriptorSetLayout);

            auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
            if (createPipelineLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->specularReflectionPipelineLayout = std::move(createdPipelineLayout);
        }

        // TODO result
        context->specularReflectionPipelineState = CreateSpecularReflectionPipelineState(context.get());

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<SpecularReflectionTransientContext>> SpecularReflectionRenderPass::PrepareSpecularReflectionTransientContext(
        SpecularReflectionPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<SpecularReflectionTransientContext>();
        context->descriptorPool = descriptorPool;

        {
            auto [result, allocatedDescriptorSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->cubemapDescriptorSetLayout);

            if (result != GraphicsResult::Success)
            {
                return { false };
            }

            context->cubemapDescriptorSet = std::move(allocatedDescriptorSet);
        }

        {
            auto [result, allocatedDescriptorSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->paramsBufferDescriptorSetLayout);

            if (result != GraphicsResult::Success)
            {
                return { false };
            }

            context->paramsBufferDescriptorSet = std::move(allocatedDescriptorSet);
        }

        return { true, std::move(context) };
    }
}
