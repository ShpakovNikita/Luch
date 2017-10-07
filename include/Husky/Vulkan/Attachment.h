#pragma once

#include <Husky/Types.h>
#include <Husky/Format.h>

namespace Husky::Vulkan
{
    class RenderPass;

    class Attachment
    {
        friend class RenderPassCreateInfo;
    public:
        inline Format GetFormat() const { return format; }
        inline Format SetFormat(Format aFormat) { format = aFormat; }

    private:
        int32 index = -1;
        Format format = Format::Undefined;
        int32 sampleCount = 1;
        vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare;
        vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined;
    };
}
