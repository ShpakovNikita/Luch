#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphNodeBuilder
    {
    public:
        RenderGraphNodeBuilder(
            RenderGraphBuilder* graphBuilder,
            int32 nodeIndex,
            RenderGraphResourceManager* resourceManager);

        RenderMutableResource ImportColorAttachment(int32 index, RefPtr<Texture> texture);
        RenderMutableResource ImportDepthStencilAttachment(RefPtr<Texture> texture);

        RenderMutableResource CreateColorAttachment(int32 index, Size2i size);
        RenderMutableResource CreateDepthStencilAttachment(Size2i size);

        RenderMutableResource WritesToColorAttachment(int32 index, RenderMutableResource resource);
        RenderMutableResource WritesToDepthStencilAttachment(RenderMutableResource resource);

        RenderResource ReadsTexture(RenderResource resource);

        RenderMutableResource ImportBuffer(RefPtr<Buffer> buffer);
        RenderMutableResource CreateBuffer(const BufferCreateInfo& createInfo);
        RenderMutableResource WritesToBuffer(RenderMutableResource resource);
        RenderResource ReadsBuffer(RenderResource resource);
    private:
        RenderGraphNode* GetNode() const;
        RenderGraphBuilder* graphBuilder = nullptr;
        int32 nodeIndex = 0;
        RenderGraphResourceManager* resourceManager = nullptr;
    };
}
