#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/Attachment.h>

namespace Luch::Graphics
{
    struct FrameBufferAttachment
    {
        Texture* texture = nullptr;
        int32 mipmapLevel = 0;
        int32 slice = 0;
        int32 depthPlane = 0;
    };

    struct FrameBufferCreateInfo
    {
        static constexpr int32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        RefPtr<RenderPass> renderPass = nullptr;
        Array<Texture*, MaxColorAttachmentCount> colorTextures;
        Texture* depthStencilTexture;
    };
}
