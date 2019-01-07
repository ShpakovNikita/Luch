#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Vulkan/VulkanRenderPass.h>

namespace Luch::Vulkan
{
    class VulkanAttachment
    {
        friend class VulkanRenderPassCreateInfo;
    public:
        inline int32 GetIndex() const { return index; }

        inline Graphics::Format GetFormat() const { return format; }

        inline VulkanAttachment& SetFormat(Graphics::Format aFormat)
        {
            format = aFormat;
            return *this;
        }

//        inline SampleCount GetSampleCount() const { return sampleCount; }

//        inline Attachment& SetSampleCount(SampleCount aSampleCount)
//        {
//            sampleCount = aSampleCount;
//            return *this;
//        }

        inline vk::AttachmentLoadOp GetLoadOp() const { return loadOp; }

        inline VulkanAttachment& SetLoadOp(vk::AttachmentLoadOp aLoadOp)
        {
            loadOp = aLoadOp;
            return *this;
        }

        inline vk::AttachmentStoreOp GetStoreOp() const { return storeOp; }

        inline VulkanAttachment& SetStoreOp(vk::AttachmentStoreOp aStoreOp)
        {
            storeOp = aStoreOp;
            return *this;
        }

        inline vk::AttachmentLoadOp GetStencilLoadOp() const { return stencilLoadOp; }

        inline VulkanAttachment& SetStencilLoadOp(vk::AttachmentLoadOp aLoadOp)
        {
            stencilLoadOp = aLoadOp;
            return *this;
        }

        inline vk::AttachmentStoreOp GetStencilStoreOp() const { return stencilStoreOp; }

        inline VulkanAttachment& SetStencilStoreOp(vk::AttachmentStoreOp aStoreOp)
        {
            stencilStoreOp = aStoreOp;
            return *this;
        }

        inline vk::ImageLayout GetInitialLayout() const { return initialLayout; }

        inline VulkanAttachment& SetInitialLayout(vk::ImageLayout layout)
        {
            initialLayout = layout;
            return *this;
        }

        inline vk::ImageLayout GetFinalLayout() const { return finalLayout; }

        inline VulkanAttachment& SetFinalLayout(vk::ImageLayout layout)
        {
            finalLayout = layout;
            return *this;
        }
    private:
        int32 index = -1;
        Graphics::Format format = Graphics::Format::Undefined;
        // SampleCount sampleCount = SampleCount::e1;
        vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare;
        vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined;
    };
}
