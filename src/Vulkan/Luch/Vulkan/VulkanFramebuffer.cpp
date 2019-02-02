#include <Luch/Vulkan/VulkanFramebuffer.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanFramebuffer::VulkanFramebuffer(
        VulkanGraphicsDevice* aDevice,
        vk::Framebuffer aFramebuffer)
        : FrameBuffer (aDevice)
        , device(aDevice)
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
