#pragma once

#include <Luch/Assert.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Attachment.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanSubpassDescription
    {
        friend class RenderPassCreateInfo;
    public:
        inline int32 GetIndex() const { return index; }

        inline VulkanSubpassDescription& WithNInputAttachments(int32 inputAttachmentCount)
        {
            inputAttachments.reserve(inputAttachmentCount);
            return *this;
        }

        inline VulkanSubpassDescription& WithNColorAttachments(int32 colorAttachmentCount)
        {
            colorAttachments.reserve(colorAttachmentCount);
            return *this;
        }

        inline VulkanSubpassDescription& WithNPreserveAttachments(int32 preserveAttachmentCount)
        {
            preserveAttachments.reserve(preserveAttachmentCount);
            return *this;
        }

        inline VulkanSubpassDescription& AddInputAttachment(const Attachment* attachment, vk::ImageLayout layout)
        {
            inputAttachments.push_back({ attachment, layout });
            return *this;
        }

        inline VulkanSubpassDescription& AddColorAttachment(const Attachment* attachment, vk::ImageLayout layout)
        {
            SubpassAttachmentReference colorAttachment = { attachment, layout };
            SubpassAttachmentReference resolveAttachment = { nullptr, vk::ImageLayout::eUndefined };

            colorAttachments.push_back({ colorAttachment, resolveAttachment });

            return *this;
        }

        inline VulkanSubpassDescription& AddColorAttachmentWithResolve(
            const Attachment* colorAttachment,
            vk::ImageLayout colorAttachmentLayout,
            const Attachment* resolveAttachment,
            vk::ImageLayout resolveAttachmentLayout)
        {
            LUCH_ASSERT_MSG(resolveAttachment != nullptr, "Null resolve attachment");

            SubpassAttachmentReference subpassColorAttachment = { colorAttachment, colorAttachmentLayout };
            SubpassAttachmentReference subpassResolveAttachment = { resolveAttachment, resolveAttachmentLayout };

            colorAttachments.push_back({ subpassColorAttachment, subpassResolveAttachment });
            return *this;
        }

        inline VulkanSubpassDescription& AddPreserveAttachment(const Attachment* attachment)
        {
            preserveAttachments.push_back(attachment);
            return *this;
        }

        inline VulkanSubpassDescription& WithDepthStencilAttachment(const Attachment* attachment, vk::ImageLayout layout)
        {
            depthStencilAttachment = { attachment, layout };
            return *this;
        }
    private:
        struct SubpassAttachmentReference
        {
            const Attachment* attachment = nullptr;
            vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        };

        struct ColorAttachmentReference
        {
            SubpassAttachmentReference colorAttachment;
            SubpassAttachmentReference resolveAttachment;
        };

        int32 index = -1;
        Vector<SubpassAttachmentReference> inputAttachments;
        Vector<ColorAttachmentReference> colorAttachments;
        Vector<const Attachment*> preserveAttachments;
        SubpassAttachmentReference depthStencilAttachment;
    };
}
