#include <Luch/Vulkan/RenderPass/VulkanRenderPass.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/Common/VulkanFormat.h>

namespace Luch::Vulkan
{
    VulkanRenderPass::VulkanRenderPass(
        VulkanGraphicsDevice* aDevice,
        vk::RenderPass aRenderPass,
        int32 aAttachmentCount)
        : RenderPass(aDevice)
        , device(aDevice)
        , renderPass(aRenderPass)
        , attachmentCount(aAttachmentCount)
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        Destroy();
    }

    const RenderPassCreateInfo& VulkanRenderPass::GetCreateInfo() const
    {
        return createInfoUnused;
    }

    void VulkanRenderPass::Destroy()
    {
        if (device)
        {
            device->DestroyRenderPass(this);
        }
    }
}
