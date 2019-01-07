#include <Luch/Vulkan/FramebufferCreateInfo.h>
#include <Luch/Vulkan/VulkanRenderPass.h>

namespace Luch::Vulkan
{
    FramebufferCreateInfo::FramebufferCreateInfo(
        VulkanRenderPass* aRenderPass,
        int32 aWidth,
        int32 aHeight,
        int32 aLayers)
        : renderPass(aRenderPass)
        , width(aWidth)
        , height(aHeight)
        , layers(aLayers)
    {
        attachments.resize(renderPass->GetAttachmentCount());
    }

    vk::FramebufferCreateInfo FramebufferCreateInfo::ToVulkanCreateInfo(const FramebufferCreateInfo& createInfo)
    {
        vk::FramebufferCreateInfo vkci;
        vkci.setRenderPass(createInfo.renderPass->GetRenderPass());
        vkci.setAttachmentCount((int32)createInfo.attachments.size());
        vkci.setPAttachments(createInfo.attachments.data());
        vkci.setWidth(createInfo.width);
        vkci.setHeight(createInfo.height);
        vkci.setLayers(createInfo.layers);
        return vkci;
    }
}
