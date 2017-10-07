#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/ImageView.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class RenderPass;

    class FramebufferCreateInfo
    {
    public:
        static vk::FramebufferCreateInfo ToVulkanCreateInfo(const FramebufferCreateInfo& createInfo);

        FramebufferCreateInfo(RenderPass* aRenderPass, int32 aWidth, int32 aHeight, int32 aLayers);

        inline FramebufferCreateInfo& WithNAttachments(int32 attachmentCount)
        {
            attachments.reserve(attachmentCount);
            return *this;
        }

        inline FramebufferCreateInfo& AddAtachment(ImageView* attachment)
        {
            attachments.push_back(attachment->GetImageView());
            return *this;
        }
    private:
        RenderPass* renderPass = nullptr;
        Vector<vk::ImageView> attachments;
        int32 width = 0;
        int32 height = 0;
        int32 layers = 0;
    };

    class Framebuffer
    {
        friend class GraphicsDevice;
    public:
        Framebuffer() = default;

        Framebuffer(Framebuffer&& other);
        Framebuffer& operator=(Framebuffer&& other);

        ~Framebuffer();

        vk::Framebuffer GetPFramebuffer() { return framebuffer; }
    private:
        Framebuffer(GraphicsDevice* device, vk::Framebuffer framebuffer);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Framebuffer framebuffer;
    };
}
