#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphAttachmentCreateInfo.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/ResourceStorageMode.h>

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

        RenderMutableResource UseColorAttachment(int32 index, RenderMutableResource colorAttachmentHandle);
        RenderMutableResource UseDepthStencilAttachment(RenderMutableResource depthStencilAttachmentHandle);

        RenderMutableResource CreateColorAttachment(int32 index, const RenderGraphAttachmentCreateInfo& createInfo);
        RenderMutableResource CreateDepthStencilAttachment(const RenderGraphAttachmentCreateInfo& createInfo);

        RenderMutableResource WritesToColorAttachment(int32 index, RenderMutableResource resource);
        RenderMutableResource WritesToDepthStencilAttachment(RenderMutableResource resource);

        RenderMutableResource ImportTexture(RefPtr<Texture> texture);
        RenderMutableResource CreateTexture(const TextureCreateInfo& createInfo);
        RenderMutableResource WritesToTexture(RenderMutableResource resource);
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
