#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Graphics/Rect2.h>
#include <Luch/Graphics/Viewport.h>
#include <Luch/Graphics/Color.h>
#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class GraphicsCommandList : public GraphicsObject
    {
    public:
        GraphicsCommandList(GraphicsDevice* device) : GraphicsObject(device) {}

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void SetLabel(const String& label) = 0;

        virtual void BeginRenderPass(FrameBuffer* framebuffer) = 0;
        virtual void EndRenderPass() = 0;

        virtual void BindGraphicsPipelineState(GraphicsPipelineState* pipelineState) = 0;

        virtual void BindTextureDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) = 0;

        virtual void BindBufferDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) = 0;

        virtual void BindSamplerDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) = 0;

        virtual void BindVertexBuffers(
            const ArrayProxy<Buffer*>& buffers,
            const ArrayProxy<int32>& offsets) = 0;

        virtual void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType,
            int32 indexBufferOffset) = 0;

        virtual void SetViewports(const ArrayProxy<Viewport>& viewports) = 0;
        virtual void SetScissorRects(const ArrayProxy<Rect2i>& scissorRects) = 0;

        virtual void Draw(
            int32 vertexStart,
            int32 vertexCount) = 0;

        virtual void DrawInstanced(
            int32 vertexStart,
            int32 vertexCount,
            int32 instanceCount,
            int32 baseInstance) = 0;

        virtual void DrawIndexedInstanced(
            int32 indexCount,
            int32 baseVertex,
            int32 instanceCount,
            int32 baseInstance) = 0;
    };
}
