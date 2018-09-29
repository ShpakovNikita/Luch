#include <Husky/Vulkan/VulkanRenderPass.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>
#include <Husky/Vulkan/VulkanFormat.h>
#include <Husky/Vulkan/VulkanSampleCount.h>

namespace Husky::Vulkan
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
