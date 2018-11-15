#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Graphics/Rect.h>
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

        virtual void BeginRenderPass(const RenderPassCreateInfo& renderPassCreateInfo) = 0;
        virtual void EndRenderPass() = 0;

        virtual void BindPipelineState(PipelineState* pipelineState) = 0;

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
            const ArrayProxy<int32>& offsets,
            int32 firstBuffer = 0) = 0;

        virtual void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType,
            int32 indexBufferOffset) = 0;

        virtual void SetViewports(const ArrayProxy<Viewport>& viewports) = 0;
        virtual void SetScissorRects(const ArrayProxy<IntRect>& scissorRects) = 0;

//        virtual void SetDepthBias(
//            float32 depthBias,
//            float32 slopeScale,
//            float32 clamp) = 0;
//
//        virtual void SetBlendColor(ColorSNorm32 color) = 0;
//        virtual void SetStencilReference(uint32 front, uint32 back) = 0;

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

        // TODO Indirect Draw
    };
}
