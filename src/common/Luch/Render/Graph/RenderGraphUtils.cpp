#include <Luch/Render/Graph/RenderGraphUtils.h>
#include <Luch/Render/Graph/RenderGraphPassAttachmentConfig.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Graphics/RenderPass.h>

namespace Luch::Render::Graph::Utils
{
    using namespace Graphics;

    void PopulateAttachmentConfig(RenderGraphPassAttachmentConfig& attachmentConfig, RenderPass* renderPass)
    {
        const auto& renderPassCreateInfo = renderPass->GetCreateInfo();

        for(uint32 i = 0; i < renderPassCreateInfo.colorAttachments.size(); i++)
        {
            if(renderPassCreateInfo.colorAttachments[i].has_value())
            {
                attachmentConfig.colorAttachments[i].emplace();
            }
        }

        if(renderPassCreateInfo.depthStencilAttachment.has_value())
        {
            attachmentConfig.depthStencilAttachment.emplace();
        }
    }

    PopulateAttachmentsResult PopulateAttachments(RenderGraphNodeBuilder* node, RenderGraphPassAttachmentConfig& attachmentConfig)
    {
        PopulateAttachmentsResult result;

        node->SetAttachmentSize(attachmentConfig.attachmentSize);

        if(attachmentConfig.depthStencilAttachment.has_value())
        {
            if(attachmentConfig.depthStencilAttachment->resource.has_value())
            {
                result.depthStencilTextureHandle = node->UseDepthStencilAttachment(
                    *attachmentConfig.depthStencilAttachment->resource,
                    attachmentConfig.depthStencilAttachment->descriptor);
            }
            else
            {
                result.depthStencilTextureHandle = node->CreateDepthStencilAttachment(
                    attachmentConfig.depthStencilAttachment->createInfo.value_or(RenderGraphAttachmentCreateInfo{}),
                    attachmentConfig.depthStencilAttachment->descriptor);
            }
        }

        for(uint32 i = 0; i < RenderGraphPassAttachmentConfig::MaxColorAttachmentCount; i++)
        {
            if(!attachmentConfig.colorAttachments[i].has_value())
            {
                continue;
            }

            auto& attachment = *attachmentConfig.colorAttachments[i];

            if(attachment.resource.has_value())
            {
                result.colorTextureHandles[i] = node->UseColorAttachment(
                    i,
                    *attachment.resource,
                    attachment.descriptor);
            }
            else
            {
                result.colorTextureHandles[i] = node->CreateColorAttachment(
                    i,
                    attachment.createInfo.value_or(RenderGraphAttachmentCreateInfo{}),
                    attachment.descriptor);
            }
        }

        return result;
    }
}
