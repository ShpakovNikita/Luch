#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VuklanAttachment.h>
#include <Luch/Vulkan/ImageView.h>

namespace Luch::Vulkan
{
    class FramebufferCreateInfo
    {
    public:
        static vk::FramebufferCreateInfo ToVulkanCreateInfo(const FramebufferCreateInfo& createInfo);

        FramebufferCreateInfo(RenderPass* aRenderPass, int32 aWidth, int32 aHeight, int32 aLayers);

        inline FramebufferCreateInfo& AddAttachment(Attachment* attachment, ImageView* imageView)
        {
            attachments[attachment->GetIndex()] = imageView->GetImageView();
            return *this;
        }
    private:
        RenderPass * renderPass = nullptr;
        Vector<vk::ImageView> attachments;
        int32 width = 0;
        int32 height = 0;
        int32 layers = 0;
    };
}
