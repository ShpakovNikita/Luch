#pragma once

#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalGraphicsCommandList : public GraphicsCommandList
    {
    public:
        MetalGraphicsCommandList(
            MetalCommandQueue* queue,
            mtlpp::CommandBuffer commandBuffer);

        void Begin(const RenderPassCreateInfo& renderPassCreateInfo) override;
        void End() override;

        void BindPipelineState(PipelineState* pipelineState) override;
        void BindTextureDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet) override;
        void BindBufferDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet) override;
        void BindSamplerDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet) override;

        void BindVertexBuffers(
            const ArrayProxy<Buffer*>& buffers,
            const ArrayProxy<int32> offsets,
            int32 firstBuffer = 0) override;

        virtual void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType) = 0;
    private:
        mtlpp::CommandBuffer commandBuffer;
        mtlpp::RenderCommandEncoder commandEncoder;
    };
}
