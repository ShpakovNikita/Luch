#include <Luch/Vulkan/VulkanImageView.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanImageView::VulkanImageView(
        VulkanGraphicsDevice* aDevice,
        vk::ImageView aImageView)
        : device(aDevice)
        , imageView(aImageView)
    {
    }

    VulkanImageView::~VulkanImageView()
    {
        Destroy();
    }

    void VulkanImageView::Destroy()
    {
        if (device)
        {
            device->DestroyImageView(this);
        }
    }
}
