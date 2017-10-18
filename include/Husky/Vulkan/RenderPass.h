#pragma once

#include <Husky/Assert.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Attachment.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class RenderPassCreateInfo;

    class SubpassDescription
    {
        friend class RenderPassCreateInfo;
    public:
        inline int32 GetIndex() const { return index; }

        inline SubpassDescription& WithNInputAttachments(int32 inputAttachmentCount)
        {
            inputAttachments.reserve(inputAttachmentCount);
            return *this;
        }

        inline SubpassDescription& WithNColorAttachments(int32 colorAttachmentCount)
        {
            colorAttachments.reserve(colorAttachmentCount);
            return *this;
        }

        inline SubpassDescription& WithNPreserveAttachments(int32 preserveAttachmentCount)
        {
            preserveAttachments.reserve(preserveAttachmentCount);
            return *this;
        }

        inline SubpassDescription& AddInputAttachment(const Attachment* attachment, vk::ImageLayout layout)
        {
            inputAttachments.push_back({ attachment, layout });
            return *this;
        }

        inline SubpassDescription& AddColorAttachment(const Attachment* attachment, vk::ImageLayout layout)
        {
            SubpassAttachmentReference colorAttachment = { attachment, layout };
            SubpassAttachmentReference resolveAttachment = { nullptr, vk::ImageLayout::eUndefined };

            colorAttachments.push_back({ colorAttachment, resolveAttachment });

            return *this;
        }

        inline SubpassDescription& AddColorAttachmentWithResolve(
            const Attachment* colorAttachment,
            vk::ImageLayout colorAttachmentLayout,
            const Attachment* resolveAttachment,
            vk::ImageLayout resolveAttachmentLayout)
        {
            HUSKY_ASSERT(resolveAttachment != nullptr, "Null resolve attachment");

            SubpassAttachmentReference subpassColorAttachment = { colorAttachment, colorAttachmentLayout };
            SubpassAttachmentReference subpassResolveAttachment = { resolveAttachment, resolveAttachmentLayout };

            colorAttachments.push_back({ subpassColorAttachment, subpassResolveAttachment });
        }

        inline SubpassDescription& AddPreserveAttachment(const Attachment* attachment)
        {
            preserveAttachments.push_back(attachment);
            return *this;
        }

        inline SubpassDescription& WithDepthStencilAttachment(const Attachment* attachment, vk::ImageLayout layout)
        {
            depthStencilAttachment = { attachment, layout };
            return *this;
        }
    private:
        struct SubpassAttachmentReference
        {
            const Attachment* attachment;
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

    struct SubpassDependency
    {
        SubpassDescription* srcSubpass;
        SubpassDescription* dstSubpass;
        vk::PipelineStageFlags srcStageMask;
        vk::PipelineStageFlags dstStageMask;
        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;
        bool byRegion;
    };

    class RenderPassCreateInfo
    {
    public:
        struct VulkanSubpassDescription
        {
            VulkanSubpassDescription() = default;

            VulkanSubpassDescription(const VulkanSubpassDescription& other) = delete;
            VulkanSubpassDescription& operator=(const VulkanSubpassDescription& other) = delete;

            VulkanSubpassDescription(VulkanSubpassDescription&& other) = default;
            VulkanSubpassDescription& operator=(VulkanSubpassDescription&& other) = default;

            vk::SubpassDescription subpass;
            Vector<vk::AttachmentReference> inputAttachments;
            Vector<vk::AttachmentReference> colorAttachments;
            Vector<vk::AttachmentReference> resolveAttachments;
            Vector<uint32> preserveAttachments;
        };

        struct VulkanRenderPassCreateInfo
        {
            VulkanRenderPassCreateInfo() = default;

            VulkanRenderPassCreateInfo(const VulkanRenderPassCreateInfo& other) = delete;
            VulkanRenderPassCreateInfo& operator=(const VulkanRenderPassCreateInfo& other) = delete;

            VulkanRenderPassCreateInfo(VulkanRenderPassCreateInfo&& other) = default;
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

            HUSKY_ASSERT(attachments[attachment->index] == nullptr, "Two attachments are using same index");

            attachments[attachment->index] = attachment;

            return *this;
        }

        inline RenderPassCreateInfo& AddSubpass(SubpassDescription&& subpassDescription)
        {
            HUSKY_ASSERT(subpassDescription.index < 0, "Can't use subpass description multiple times");
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

    class RenderPass
    {
        friend class GraphicsDevice;
    public:
        RenderPass() = default;
        RenderPass(RenderPass&& other);
        RenderPass& operator=(RenderPass&& other);
        ~RenderPass();
        inline vk::RenderPass GetRenderPass() { return renderPass; }
        inline int32 GetAttachmentCount() const { return attachmentCount; }
    private:
        void Destroy();

        RenderPass(GraphicsDevice* device, vk::RenderPass renderPass, int32 attachmentCount);
        GraphicsDevice* device = nullptr;
        vk::RenderPass renderPass;
        int32 attachmentCount = 0;
    };
}
