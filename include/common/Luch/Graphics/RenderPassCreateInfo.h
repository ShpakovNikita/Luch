#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Attachment.h>

namespace Luch::Graphics
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

        RenderPassCreateInfo& WithName(const String& aName)
        {
            name = aName;
            return *this;
        }

        int32 currentColorAttachmentIndex = 0;
        int32 totalColorAttachmentsCount = 0;
        Vector<ColorAttachment*> colorAttachments;
        DepthStencilAttachment* depthStencilAttachment = nullptr;
        String name = "";
    };
}
