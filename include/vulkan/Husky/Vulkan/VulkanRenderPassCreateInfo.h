#pragma once

#include <Husky/UniquePtr.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Attachment.h>
#include <Husky/Vulkan/SubpassDescription.h>
#include <Husky/Vulkan/SubpassDependency.h>

namespace Husky::Vulkan
{
    class VulkanRenderPassCreateInfo
    {
    public:
        struct VulkanSubpassDescription
        {
            VulkanSubpassDescription() = default;

            VulkanSubpassDescription(const VulkanSubpassDescription& other) = delete;
            VulkanSubpassDescription(VulkanSubpassDescription&& other) = default;

            VulkanSubpassDescription& operator=(const VulkanSubpassDescription& other) = delete;
            VulkanSubpassDescription& operator=(VulkanSubpassDescription&& other) = default;

            vk::SubpassDescription subpass;
            Vector<vk::AttachmentReference> inputAttachments;
            Vector<vk::AttachmentReference> colorAttachments;
            Vector<vk::AttachmentReference> resolveAttachments;
            UniquePtr<vk::AttachmentReference> depthStencilAttachment;
            Vector<uint32> preserveAttachments;
        };

        struct VulkanRenderPassCreateInfo
        {
            VulkanRenderPassCreateInfo() = default;

            VulkanRenderPassCreateInfo(const VulkanRenderPassCreateInfo& other) = delete;
            VulkanRenderPassCreateInfo(VulkanRenderPassCreateInfo&& other) = default;

            VulkanRenderPassCreateInfo& operator=(const VulkanRenderPassCreateInfo& other) = delete;
            VulkanRenderPassCreateInfo& operator=(VulkanRenderPassCreateInfo&& other) = default;

            vk::RenderPassCreateInfo createInfo;
            Vector<vk::AttachmentDescription> attachments;
            Vector<VulkanSubpassDescription> subpassesInfo;
            Vector<vk::SubpassDescription> subpasses;
            Vector<vk::SubpassDependency> subpassDependencies;
        };

        static VulkanRenderPassCreateInfo ToVulkanCreateInfo(const RenderPassCreateInfo& createInfo);

        // attachment must be in a valid state
        // changing the attachment after calling this function can lead to undefined behavior
        inline RenderPassCreateInfo& AddAttachment(Attachment* attachment)
        {
            if (attachment->index < 0)
            {
                attachment->index = nextAttachmentIndex;
                nextAttachmentIndex++;
            }

            if (attachment->index >= attachments.size())
            {
                attachments.resize(attachment->index + 1);
            }

            HUSKY_ASSERT_MSG(attachments[attachment->index] == nullptr, "Two attachments are using same index");

            attachments[attachment->index] = attachment;

            return *this;
        }

        inline RenderPassCreateInfo& AddSubpass(SubpassDescription&& subpassDescription)
        {
            HUSKY_ASSERT_MSG(subpassDescription.index < 0, "Can't use subpass description multiple times");
            subpassDescription.index = nextSubpassIndex;
            nextSubpassIndex++;

            subpasses.emplace_back(subpassDescription);

            return *this;
        }

        inline RenderPassCreateInfo& AddSubpassDependency(const SubpassDependency& dependency)
        {
            subpassDependencies.push_back(dependency);
            return *this;
        }
    private:
        static vk::AttachmentDescription ToVulkanAttachmentDescription(const Attachment* attachment);
        static VulkanSubpassDescription ToVulkanSubpassDescription(const SubpassDescription& subpassDescription);
        static vk::SubpassDependency ToVulkanSubpassDependency(const SubpassDependency& subpassDependency);

        int32 nextAttachmentIndex = 0;
        int32 nextSubpassIndex = 0;
        Vector<Attachment*> attachments;
        Vector<SubpassDescription> subpasses;
        Vector<SubpassDependency> subpassDependencies;
    };
}
