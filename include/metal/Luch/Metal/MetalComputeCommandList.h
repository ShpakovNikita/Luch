#pragma once

#include <Luch/Graphics/ComputeCommandList.h>
#include <Luch/Metal/MetalForwards.h>
#include <Luch/Metal/MetalCommandList.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalComputeCommandList 
        : public ComputeCommandList
        , public MetalCommandList
    {
        friend class MetalCommandQueue;
    public:
        MetalComputeCommandList(
            MetalGraphicsDevice* device,
            mtlpp::CommandBuffer commandBuffer);

        void Begin() override;
        void End() override;

        void SetLabel(const String& label) override;

        void BindPipelineState(ComputePipelineState* pipelineState) override;

        void BindTextureDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindBufferDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindSamplerDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void DispatchThreadgroups(
            Size3i threadgroupsPerGrid,
            Size3i threadsPerThreadgroup) override;
    private:
        mtlpp::ComputeCommandEncoder commandEncoder;
        String label;
    };
}


