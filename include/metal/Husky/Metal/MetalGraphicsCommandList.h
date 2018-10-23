#pragma once

#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Metal/MetalForwards.h>

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

        inline void Begin() override {};
        inline void End() override {};

        void BeginRenderPass(const RenderPassCreateInfo& renderPassCreateInfo) override;
        void EndRenderPass() override;

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
            IndexType indexType,
            int32 indexBufferOffset) override;

        void SetViewports(const ArrayProxy<Viewport>& viewports) override;
        void SetScissorRects(const ArrayProxy<IntRect>& scissorRects) override;

        void Draw(
            int32 vertexStart,
            int32 vertexCount) override;

        void DrawInstanced(
            int32 vertexStart,
            int32 vertexCount,
            int32 instanceCount,
            int32 instanceBase) override;

        void DrawIndexedInstanced(
            int32 indexCount,
            int32 baseVertex,
            int32 instanceCount,
            int32 baseInstance) override;
    private:
        PipelineState* pipelineState = nullptr;
        mtlpp::CommandBuffer commandBuffer;
        mtlpp::RenderCommandEncoder commandEncoder;
        mtlpp::Buffer indexBuffer;
        mtlpp::IndexType indexType = mtlpp::IndexType::UInt16;
        int32 indexBufferOffset;
        mtlpp::PrimitiveType primitiveType = mtlpp::PrimitiveType::Triangle;
    };
}
