#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    struct RenderGraphNode
    {
        static constexpr int32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        RenderGraphPass* pass = nullptr;
        String name;
        Array<Optional<ColorAttachment>, MaxColorAttachmentCount> colorAttachments;
        Optional<DepthStencilAttachment> depthStencilAttachment;
        Array<RenderMutableResource, MaxColorAttachmentCount> colorAttachmentResources;
        RenderMutableResource depthStencilAttachmentResource;
        Vector<RenderMutableResource> importedResources;
        Vector<RenderMutableResource> createdResources;
        Vector<RenderMutableResource> writtenResources;
        Vector<RenderResource> readResources;
        RefPtr<RenderPass> renderPass;
        RefPtr<FrameBuffer> frameBuffer;
    };
}
