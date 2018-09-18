#include <Husky/Vulkan/VulkanFramebuffer.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
{
    VulkanFramebuffer::VulkanFramebuffer(
        VulkanGraphicsDevice* aDevice,
        vk::Framebuffer aFramebuffer)
        : device(aDevice)
        , framebuffer(aFramebuffer)
    {
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        Destroy();
    }

    void VulkanFramebuffer::Destroy()
    {
        if (device)
        {
            device->DestroyFramebuffer(this);
        }
    }
}
