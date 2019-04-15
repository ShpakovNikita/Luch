#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Size2.h>
#include <Luch/Render/Graph/RenderGraphAttachment.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphPassType.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Render/Graph/RenderGraphAttachmentCreateInfo.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    struct RenderGraphPassAttachmentConfig
    {
        struct Attachment
        {
            Optional<RenderMutableResource> resource;
            RenderGraphAttachmentDescriptor descriptor;
            Optional<RenderGraphAttachmentCreateInfo> createInfo; 
        };

        static constexpr uint32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        Size2i attachmentSize;
        Array<Optional<Attachment>, MaxColorAttachmentCount> colorAttachments = {};
        Optional<Attachment> depthStencilAttachment;
    };
}
