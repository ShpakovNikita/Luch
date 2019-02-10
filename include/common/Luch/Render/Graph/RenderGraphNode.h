#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphPassType.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    struct RenderGraphAttachment
    {
        RenderMutableResource resource;
        int32 mipmapLevel = 0;
        int32 slice = 0;
        int32 depthPlane = 0;
    };

    struct RenderGraphNode
    {
        static constexpr int32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        RenderGraphPass* pass = nullptr;
        String name;
        Array<RenderGraphAttachment, MaxColorAttachmentCount> colorAttachments = {};
        RenderGraphAttachment depthStencilAttachment;
        Vector<RenderMutableResource> importedResources;
        Vector<RenderMutableResource> createdResources;
        Vector<RenderMutableResource> writtenResources;
        Vector<RenderResource> readResources;
        RefPtr<RenderPass> renderPass;
        RefPtr<FrameBuffer> frameBuffer;
        RenderGraphPassType type = RenderGraphPassType::Graphics;
    };
}
