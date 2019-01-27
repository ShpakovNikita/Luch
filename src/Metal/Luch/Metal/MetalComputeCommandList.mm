#include <Luch/Metal/MetalComputeCommandList.h>
#include <Luch/Metal/MetalComputePipelineState.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalPipelineLayout.h>
#include <Luch/Metal/MetalDescriptorSet.h>
#include <Luch/Metal/MetalDescriptorSetLayout.h>
#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalTexture.h>
#include <Luch/Assert.h>

using namespace Luch::Graphics;

namespace Luch::Metal
{
    MetalComputeCommandList::MetalComputeCommandList(
        MetalGraphicsDevice* device,
        mtlpp::CommandBuffer aCommandBuffer)
        : ComputeCommandList(device)
        , MetalCommandList(aCommandBuffer)
    {
    }

    void MetalComputeCommandList::Begin()
    {
        commandEncoder = commandBuffer.ComputeCommandEncoder();
    }

    void MetalComputeCommandList::End()
    {
        commandEncoder.SetLabel(ns::String{ label.c_str() });
        commandEncoder.EndEncoding();
        commandEncoder = { };
    }

    void MetalComputeCommandList::SetLabel(const String& aLabel)
    {
        label = aLabel;
    }

    void MetalComputeCommandList::BindPipelineState(ComputePipelineState* pipelineState)
    {
        auto mtlPipelineState = static_cast<MetalComputePipelineState*>(pipelineState);
        commandEncoder.SetComputePipelineState(mtlPipelineState->pipelineState);
    }

    void MetalComputeCommandList::BindTextureDescriptorSet(
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);
        auto mtlDescriptorSetLayout = mtlDescriptorSet->descriptorSetLayout;
        auto mtlPipelineLayout = static_cast<MetalPipelineLayout*>(pipelineLayout);
        auto textureSetLayouts = mtlPipelineLayout->createInfo.stages[ShaderStage::Compute].textureSetLayouts;
        auto layoutIt = std::find(textureSetLayouts.begin(), textureSetLayouts.end(), mtlDescriptorSetLayout);
        LUCH_ASSERT(layoutIt != textureSetLayouts.end());

        uint32 start = 0;
        for(auto it = textureSetLayouts.begin(); it != layoutIt; it++)
        {
            auto mtlLayout = static_cast<MetalDescriptorSetLayout*>(*it);
            start += mtlLayout->createInfo.bindings.size();
        }

        LUCH_ASSERT(mtlDescriptorSetLayout->createInfo.type == DescriptorSetType::Texture);
        auto textures = mtlDescriptorSet->textures.data();
        auto length = (uint32)mtlDescriptorSet->textures.size();

        LUCH_ASSERT(length != 0);
        auto range = ns::Range { start, length };

        commandEncoder.SetTextures(textures, range);
    }

    void MetalComputeCommandList::BindBufferDescriptorSet(
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);
        auto mtlDescriptorSetLayout = mtlDescriptorSet->descriptorSetLayout;
        auto mtlPipelineLayout = static_cast<MetalPipelineLayout*>(pipelineLayout);
        auto bufferSetLayouts = mtlPipelineLayout->createInfo.stages[ShaderStage::Compute].bufferSetLayouts;
        auto layoutIt = std::find(bufferSetLayouts.begin(), bufferSetLayouts.end(), mtlDescriptorSetLayout);
        LUCH_ASSERT(layoutIt != bufferSetLayouts.end());

        uint32 start = 0;
        for(auto it = bufferSetLayouts.begin(); it != layoutIt; it++)
        {
            auto mtlLayout = static_cast<MetalDescriptorSetLayout*>(*it);
            start += mtlLayout->createInfo.bindings.size();
        }

        LUCH_ASSERT(mtlDescriptorSetLayout->createInfo.type == DescriptorSetType::Buffer);
        auto buffers = mtlDescriptorSet->buffers.data();
        auto bufferOffsets = (uint32*)mtlDescriptorSet->bufferOffsets.data();
        auto length = (uint32)mtlDescriptorSet->buffers.size();

        LUCH_ASSERT(length != 0);
        auto range = ns::Range { start, length };

        commandEncoder.SetBuffers(buffers, bufferOffsets, range);
    }

    void MetalComputeCommandList::BindSamplerDescriptorSet(
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);
        auto mtlDescriptorSetLayout = mtlDescriptorSet->descriptorSetLayout;
        auto mtlPipelineLayout = static_cast<MetalPipelineLayout*>(pipelineLayout);
        auto samplerSetLayouts = mtlPipelineLayout->createInfo.stages[ShaderStage::Compute].samplerSetLayouts;
        auto layoutIt = std::find(samplerSetLayouts.begin(), samplerSetLayouts.end(), mtlDescriptorSetLayout);
        LUCH_ASSERT(layoutIt != samplerSetLayouts.end());

        uint32 start = 0;
        for(auto it = samplerSetLayouts.begin(); it != layoutIt; it++)
        {
            auto mtlLayout = static_cast<MetalDescriptorSetLayout*>(*it);
            start += mtlLayout->createInfo.bindings.size();
        }

        LUCH_ASSERT(mtlDescriptorSetLayout->createInfo.type == DescriptorSetType::Sampler);
        auto samplers = mtlDescriptorSet->samplers.data();
        auto length = (uint32)mtlDescriptorSet->samplers.size();

        LUCH_ASSERT(length != 0);
        auto range = ns::Range { start, length };

        commandEncoder.SetSamplerStates(samplers, range);
    }

    void MetalComputeCommandList::DispatchThreadgroups(
        Size3i threadgroupsPerGrid,
        Size3i threadsPerThreadgroup)
    {
        commandEncoder.DispatchThreadgroups(
            mtlpp::Size{
                static_cast<uint32>(threadgroupsPerGrid.width),
                static_cast<uint32>(threadgroupsPerGrid.height),
                static_cast<uint32>(threadgroupsPerGrid.depth) },
            mtlpp::Size{
                static_cast<uint32>(threadsPerThreadgroup.width),
                static_cast<uint32>(threadsPerThreadgroup.height),
                static_cast<uint32>(threadsPerThreadgroup.depth) });
    }
}

