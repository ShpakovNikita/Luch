#include <Husky/Vulkan/VulkanImageView.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
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
