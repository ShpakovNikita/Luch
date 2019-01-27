#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/Size3.h>
#include <Luch/Graphics/CommandList.h>

namespace Luch::Graphics
{
    class ComputeCommandList : public CommandList
    {
    public:
        ComputeCommandList(GraphicsDevice* device) : CommandList(device) {}

        virtual void Begin() = 0;
        virtual void End() = 0;

        CommandListType GetType() const override { return CommandListType::Compute; }

        virtual void BindPipelineState(ComputePipelineState* pipelineState) = 0;

        virtual void BindTextureDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) = 0;

        virtual void BindBufferDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) = 0;

        virtual void BindSamplerDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) = 0;

//        virtual void DispatchThreads(
//            Size2i threadsPerGrid,
//            Size2i threadsPerThreadgroup) = 0;

        virtual void DispatchThreadgroups(
            Size3i threadgroupsPerGrid,
            Size3i threadsPerThreadgroup) = 0;
    };
}
