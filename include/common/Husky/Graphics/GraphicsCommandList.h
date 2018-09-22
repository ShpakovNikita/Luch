#pragma once

#include <Husky/ArrayProxy.h>
#include <Husky/BaseObject.h>
#include <Husky/Graphics/IndexType.h>
#include <Husky/Graphics/ShaderStage.h>
#include <Husky/Graphics/Rect.h>
#include <Husky/Graphics/Viewport.h>
#include <Husky/Graphics/Color.h>
#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
{
    class GraphicsCommandList : public BaseObject
    {
    public:
        virtual ~GraphicsCommandList() = 0;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void BindPipelineState(PipelineState* pipelineState) = 0;
        virtual void BindTextureDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet) = 0;
        virtual void BindBufferDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet) = 0;
        virtual void BindSamplerDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet) = 0;

        virtual void BindVertexBuffers(
            const ArrayProxy<Buffer*>& buffers,
            const ArrayProxy<int32> offsets,
            int32 firstBuffer = 0) = 0;

        virtual void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType) = 0;

        virtual void SetViewports(const ArrayProxy<Viewport>& viewports) = 0;
        virtual void SetScissorRects(const ArrayProxy<FloatRect>& scissorRects) = 0;

//        virtual void SetDepthBias(
//            float32 depthBias,
//            float32 slopeScale,
//            float32 clamp) = 0;
//
//        virtual void SetBlendColor(ColorSNorm32 color) = 0;
//        virtual void SetStencilReference(uint32 front, uint32 back) = 0;

        virtual void Draw
            int32 vertexStart,
            int32 vertexCount) = 0;

        virtual void DrawInstanced(
            int32 vertexStart,
            int32 vertexCount,
            int32 instanceCount,
            int32 instanceBase = 0) = 0;

        virtual void DrawIndexedInstanced(
            int32 indexCount,
            int32 indexBufferOffset,
            int32 baseVertex,
            int32 instanceCount,
            int32 baseInstance = 0) = 0;

        // TODO Indirect Draw
    }

    inline GraphicsCommandList::~GraphicsCommandList() {}
}
