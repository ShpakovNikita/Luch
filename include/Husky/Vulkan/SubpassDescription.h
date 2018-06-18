#pragma once

#include <Husky/Assert.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Attachment.h>
#include <Husky/Vulkan/Forwards.h>

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
