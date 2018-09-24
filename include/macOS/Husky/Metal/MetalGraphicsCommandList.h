#pragma once

#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalGraphicsCommandList : public GraphicsCommandList
    {
        friend class MetalCommandQueue;
    public:
        MetalGraphicsCommandList(
            MetalGraphicsDevice* device,
            mtlpp::CommandBuffer commandBuffer);

        void Begin(const RenderPassCreateInfo& renderPassCreateInfo) override;
        void End() override;

        void BindPipelineState(PipelineState* pipelineState) override;

        void BindTextureDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindBufferDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindSamplerDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindVertexBuffers(
            const ArrayProxy<Buffer*>& buffers,
            const ArrayProxy<int32>& offsets,
            int32 firstBuffer = 0) override;

        void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType) override;

        void SetViewports(const ArrayProxy<Viewport>& viewports) override;
        void SetScissorRects(const ArrayProxy<IntRect>& scissorRects) override;

        void Draw(
            int32 vertexStart,
            int32 vertexCount) override;

        void DrawInstanced(
            int32 vertexStart,
            int32 vertexCount,
            int32 instanceCount,
            int32 instanceBase = 0) override;

        void DrawIndexedInstanced(
            int32 indexCount,
            int32 indexBufferOffset,
            int32 baseVertex,
            int32 instanceCount,
            int32 baseInstance = 0) override;
    private:
        PipelineState* pipelineState = nullptr;
        mtlpp::CommandBuffer commandBuffer;
        mtlpp::RenderCommandEncoder commandEncoder;
        mtlpp::Buffer indexBuffer;
        mtlpp::IndexType indexType = mtlpp::IndexType::UInt16;
        mtlpp::PrimitiveType primitiveType = mtlpp::PrimitiveType::Triangle;
    };
}
