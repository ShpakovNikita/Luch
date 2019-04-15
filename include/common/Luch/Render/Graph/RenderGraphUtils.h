#pragma once

#include <Luch/Graphics/RenderPass.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

namespace Luch::Render::Graph::Utils
{
    using namespace Graphics;

    struct PopulateAttachmentsResult
    {
        Array<RenderMutableResource, RenderPassCreateInfo::MaxColorAttachmentCount> colorTextureHandles;
        RenderMutableResource depthStencilTextureHandle;
    };

    void PopulateAttachmentConfig(RenderGraphPassAttachmentConfig& attachmentConfig, RenderPass* renderPass);
    PopulateAttachmentsResult PopulateAttachments(RenderGraphNodeBuilder* node, RenderGraphPassAttachmentConfig& attachmentConfig);
}
