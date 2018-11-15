#include <Luch/Vulkan/VulkanRenderPass.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanFormat.h>
#include <Luch/Vulkan/VulkanSampleCount.h>

namespace Luch::Vulkan
{
    VulkanRenderPass::VulkanRenderPass(
        VulkanGraphicsDevice* aDevice,
        vk::RenderPass aRenderPass,
        int32 aAttachmentCount)
        : device(aDevice)
        , renderPass(aRenderPass)
        , attachmentCount(aAttachmentCount)
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        Destroy();
    }

    void VulkanRenderPass::Destroy()
    {
        if (device)
        {
            device->DestroyRenderPass(this);
        }
    }
}
