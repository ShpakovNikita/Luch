#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/Attachment.h>

namespace Luch::Graphics
{
    struct FrameBufferCreateInfo
    {
        static constexpr int32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        RefPtr<RenderPass> renderPass = nullptr;
        Array<Texture*, MaxColorAttachmentCount> colorTextures;
        Texture* depthStencilTexture;
    };
}
