#pragma once

#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Metal/MetalForwards.h>
#include <Luch/Metal/MetalCommandList.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalGraphicsCommandList 
        : public GraphicsCommandList
        , public MetalCommandList
    {
        friend class MetalCommandQueue;
    public:
        MetalGraphicsCommandList(
            MetalGraphicsDevice* device,
            mtlpp::CommandBuffer commandBuffer);

        void SetLabel(const String& label) override;

        inline void Begin() override {};
        inline void End() override {};

        void BeginRenderPass(FrameBuffer* frameBuffer) override;
        void EndRenderPass() override;

        void BindGraphicsPipelineState(GraphicsPipelineState* pipelineState) override;
        void BindTiledPipelineState(TiledPipelineState* pipelineState) override;

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
            const ArrayProxy<int32>& offsets) override;

        void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType,
            int32 indexBufferOffset) override;

        void SetViewports(const ArrayProxy<Viewport>& viewports) override;
        void SetScissorRects(const ArrayProxy<Rect2i>& scissorRects) override;

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

        Size2i GetTileSize() const override;

        void DispatchThreadsPerTile(
            Size2i threadsPerTile) override;
    private:
        void BindBufferDescriptorSetImpl(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet);

        void BindThreadgroupDescriptorSet(
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet);

        GraphicsPipelineState* pipelineState = nullptr;
        mtlpp::RenderCommandEncoder commandEncoder;
        mtlpp::Buffer indexBuffer;
        mtlpp::IndexType indexType = mtlpp::IndexType::UInt16;
        int32 indexBufferOffset;
        mtlpp::PrimitiveType primitiveType = mtlpp::PrimitiveType::Triangle;
        String label;
    };
}
