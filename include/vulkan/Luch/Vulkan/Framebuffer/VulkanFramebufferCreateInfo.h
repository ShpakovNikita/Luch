#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Framebuffer/VulkanAttachment.h>
#include <Luch/Vulkan/Image/VulkanImageView.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>

namespace Luch::Vulkan
{
    class FramebufferCreateInfo
    {
    public:
        static vk::FramebufferCreateInfo ToVulkanCreateInfo(const FramebufferCreateInfo& createInfo);
        static vk::FramebufferCreateInfo ToVulkanCreateInfo(const Luch::Graphics::FrameBufferCreateInfo& createInfo);

        FramebufferCreateInfo(VulkanRenderPass* aRenderPass, int32 aWidth, int32 aHeight, int32 aLayers);

        inline FramebufferCreateInfo& AddAttachment(VulkanAttachment* attachment, VulkanImageView* imageView)
        {
            attachments[attachment->GetIndex()] = imageView->GetImageView();
            return *this;
        }
    private:
        VulkanRenderPass * renderPass = nullptr;
        Vector<vk::ImageView> attachments;
        int32 width = 0;
        int32 height = 0;
        int32 layers = 0;
    };
}
