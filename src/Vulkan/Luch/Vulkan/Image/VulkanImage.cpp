#include <Luch/Vulkan/Image/VulkanImage.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    constexpr ImageAspects GetAspects(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
            return ImageAspects::Depth;
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return ImageAspects::DepthStencil;
        case vk::Format::eS8Uint:
            return ImageAspects::Stencil;
        case vk::Format::eUndefined:
            return ImageAspects::None;
        default:
            return ImageAspects::Color;
        }
    }

    VulkanImage::VulkanImage(
        VulkanGraphicsDevice* aDevice,
        vk::Image aImage,
        vk::DeviceMemory aMemory,
        vk::ImageCreateInfo aCreateInfo,
        vk::MemoryRequirements aMemoryRequirements,
        bool aOwning)
        : device(aDevice)
        , image(aImage)
        , memory(aMemory)
        , createInfo(aCreateInfo)
        , aspects(GetAspects(aCreateInfo.format))
        , memoryRequirements(aMemoryRequirements)
        , owning(aOwning)
    {
    }

    VulkanImage::~VulkanImage()
    {
        Destroy();
    }

    void VulkanImage::Destroy()
    {
        if (owning && device)
        {
            device->DestroyImage(this);
        }
    }
}
