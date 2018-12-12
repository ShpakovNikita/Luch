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

        RenderMutableResource ImportColorAttachment(int32 index, RefPtr<Texture> texture, const ColorAttachment& attachment);
        RenderMutableResource ImportDepthStencilAttachment(RefPtr<Texture> texture, const DepthStencilAttachment& attachment);

        RenderMutableResource CreateColorAttachment(int32 index, int32 width, int32 height, const ColorAttachment& attachment);
        RenderMutableResource CreateDepthStencilAttachment(int32 width, int32 height, const DepthStencilAttachment& attachment);

        RenderMutableResource WritesToColorAttachment(int32 index, RenderMutableResource resource, const ColorAttachment& attachment);
        RenderMutableResource WritesToDepthStencilAttachment(RenderMutableResource resource, const DepthStencilAttachment& attachment);

        RenderResource ReadsTexture(RenderResource resource);
    private:
        RenderGraphNode* GetNode() const;
        RenderGraphBuilder* graphBuilder = nullptr;
        int32 nodeIndex = 0;
        RenderGraphResourceManager* resourceManager = nullptr;
    };
}
