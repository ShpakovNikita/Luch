#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Size2.h>
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

        void SetAttachmentSize(Size2i attachmentSize);

        RenderMutableResource ImportColorAttachment(
            int32 index,
            RefPtr<Texture> texture,
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource ImportDepthStencilAttachment(
            RefPtr<Texture> texture,
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource UseColorAttachment(
            int32 index,
            RenderMutableResource colorAttachmentHandle,
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource UseDepthStencilAttachment(
            RenderMutableResource depthStencilAttachmentHandle,
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource CreateColorAttachment(
            int32 index,
            const RenderGraphAttachmentCreateInfo& createInfo = {},
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource CreateDepthStencilAttachment(
            const RenderGraphAttachmentCreateInfo& createInfo = {},
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource WritesToColorAttachment(
            int32 index,
            RenderMutableResource resource,
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource WritesToDepthStencilAttachment(
            RenderMutableResource resource,
            const RenderGraphAttachmentDescriptor& descriptor = {});

        RenderMutableResource ImportTexture(RefPtr<Texture> texture);
        RenderMutableResource CreateTexture(const TextureCreateInfo& createInfo);
        RenderMutableResource WritesToTexture(RenderMutableResource resource);
        RenderResource ReadsTexture(RenderResource resource);

        RenderMutableResource ImportBuffer(RefPtr<Buffer> buffer);
        RenderMutableResource CreateBuffer(const BufferCreateInfo& createInfo);
        RenderMutableResource WritesToBuffer(RenderMutableResource resource);
        RenderResource ReadsBuffer(RenderResource resource);

        const RenderGraphNode* GetNode() const;
    private:
        RenderGraphNode* GetMutableNode();
        RenderGraphBuilder* graphBuilder = nullptr;
        int32 nodeIndex = 0;
        RenderGraphResourceManager* resourceManager = nullptr;
        Optional<Size2i> attachmentSize;
    };
}
