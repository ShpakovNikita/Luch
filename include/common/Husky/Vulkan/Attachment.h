#pragma once

#include <Husky/Types.h>
#include <Husky/Format.h>
#include <Husky/SampleCount.h>

namespace Husky::Vulkan
{
    class RenderPass;

    class Attachment
    {
        friend class RenderPassCreateInfo;
    public:
        inline int32 GetIndex() const { return index; }

        inline Format GetFormat() const { return format; }

        inline Attachment& SetFormat(Format aFormat)
        {
            format = aFormat;
            return *this;
        }

        inline SampleCount GetSampleCount() const { return sampleCount; }

        inline Attachment& SetSampleCount(SampleCount aSampleCount)
        {
            sampleCount = aSampleCount;
            return *this;
        }

        inline vk::AttachmentLoadOp GetLoadOp() const { return loadOp; }

        inline Attachment& SetLoadOp(vk::AttachmentLoadOp aLoadOp)
        {
            loadOp = aLoadOp;
            return *this;
        }

        inline vk::AttachmentStoreOp GetStoreOp() const { return storeOp; }

        inline Attachment& SetStoreOp(vk::AttachmentStoreOp aStoreOp)
        {
            storeOp = aStoreOp;
            return *this;
        }

        inline vk::AttachmentLoadOp GetStencilLoadOp() const { return stencilLoadOp; }

        inline Attachment& SetStencilLoadOp(vk::AttachmentLoadOp aLoadOp)
        {
            stencilLoadOp = aLoadOp;
            return *this;
        }

        inline vk::AttachmentStoreOp GetStencilStoreOp() const { return stencilStoreOp; }

        inline Attachment& SetStencilStoreOp(vk::AttachmentStoreOp aStoreOp)
        {
            stencilStoreOp = aStoreOp;
            return *this;
        }

        inline vk::ImageLayout GetInitialLayout() const { return initialLayout; }

        inline Attachment& SetInitialLayout(vk::ImageLayout layout)
        {
            initialLayout = layout;
            return *this;
        }

        inline vk::ImageLayout GetFinalLayout() const { return finalLayout; }

        inline Attachment& SetFinalLayout(vk::ImageLayout layout)
        {
            finalLayout = layout;
            return *this;
        }
    private:
        int32 index = -1;
        Format format = Format::Undefined;
        SampleCount sampleCount = SampleCount::e1;
        vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare;
        vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined;
    };
}
