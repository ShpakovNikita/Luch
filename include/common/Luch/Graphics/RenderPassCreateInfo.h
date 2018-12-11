#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Attachment.h>

namespace Luch::Graphics
{
    struct RenderPassCreateInfo
    {
        static constexpr int32 MaxColorAttachmentCount = 8;

        Array<Optional<ColorAttachment>, MaxColorAttachmentCount> colorAttachments;
        Optional<DepthStencilAttachment> depthStencilAttachment;
        String name = "";
    };
}
