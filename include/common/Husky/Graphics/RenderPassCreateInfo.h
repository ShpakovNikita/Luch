#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/Attachment.h>

namespace Husky::Graphics
{
    class RenderPassCreateInfo
    {
    public:
        RenderPassCreateInfo& WithNColorAttachments(int32 colorAttachmentsCount)
        {
            colorAttachments.reserve(colorAttachmentsCount);
            totalColorAttachmentsCount = colorAttachmentsCount;
            return *this;
        }

        RenderPassCreateInfo& AddColorAttachment(ColorAttachment* colorAttachment)
        {
            colorAttachment->index = currentColorAttachmentIndex;
            colorAttachments.push_back(colorAttachment);
            currentColorAttachmentIndex++;
            return *this;
        }

        RenderPassCreateInfo& WithDepthStencilAttachment(DepthStencilAttachment* aDepthStencilAttachment)
        {
            depthStencilAttachment = aDepthStencilAttachment;
            return *this;
        }

        int32 currentColorAttachmentIndex = 0;
        int32 totalColorAttachmentsCount = 0;
        Vector<ColorAttachment*> colorAttachments;
        DepthStencilAttachment* depthStencilAttachment = nullptr;
    };
}
