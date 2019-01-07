#pragma once

#include <Luch/UniquePtr.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanAttachment.h>
#include <Luch/Vulkan/VulkanSubpassDescription.h>
#include <Luch/Vulkan/VulkanSubpassDependency.h>

namespace Luch::Vulkan
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
            int32 index = -1;
        };

        struct VulkanRenderPassCreateInfoData
        {
            VulkanRenderPassCreateInfoData() = default;

            VulkanRenderPassCreateInfoData(const VulkanRenderPassCreateInfoData& other) = delete;
            VulkanRenderPassCreateInfoData(VulkanRenderPassCreateInfoData&& other) = default;

            VulkanRenderPassCreateInfoData& operator=(const VulkanRenderPassCreateInfoData& other) = delete;
            VulkanRenderPassCreateInfoData& operator=(VulkanRenderPassCreateInfoData&& other) = default;

            vk::RenderPassCreateInfo createInfo;
            Vector<vk::AttachmentDescription> attachments;
            Vector<VulkanSubpassDescription> subpassesInfo;
            Vector<vk::SubpassDescription> subpasses;
            Vector<vk::SubpassDependency> subpassDependencies;
        };

        static VulkanRenderPassCreateInfoData ToVulkanCreateInfo(const VulkanRenderPassCreateInfo& createInfo);

        // attachment must be in a valid state
        // changing the attachment after calling this function can lead to undefined behavior
        inline VulkanRenderPassCreateInfo& AddAttachment(VulkanAttachment* attachment)
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

            LUCH_ASSERT_MSG(attachments[attachment->index] == nullptr, "Two attachments are using same index");

            attachments[attachment->index] = attachment;

            return *this;
        }

        inline VulkanRenderPassCreateInfo& AddSubpass(VulkanSubpassDescription&& subpassDescription)
        {
            LUCH_ASSERT_MSG(subpassDescription.index < 0, "Can't use subpass description multiple times");
            subpassDescription.index = nextSubpassIndex;
            nextSubpassIndex++;

            subpasses.emplace_back(std::move(subpassDescription));

            return *this;
        }

        inline VulkanRenderPassCreateInfo& AddSubpassDependency(const VulkanSubpassDependency& dependency)
        {
            subpassDependencies.push_back(dependency);
            return *this;
        }
    private:
        static vk::AttachmentDescription ToVulkanAttachmentDescription(const VulkanAttachment* attachment);
        static VulkanSubpassDescription ToVulkanSubpassDescription(const VulkanSubpassDescription& subpassDescription);
        static vk::SubpassDependency ToVulkanSubpassDependency(const VulkanSubpassDependency& subpassDependency);

        int32 nextAttachmentIndex = 0;
        int32 nextSubpassIndex = 0;
        Vector<VulkanAttachment*> attachments;
        Vector<VulkanSubpassDescription> subpasses;
        Vector<VulkanSubpassDependency> subpassDependencies;
    };
}
