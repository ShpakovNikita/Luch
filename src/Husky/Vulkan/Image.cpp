#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
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

    Image::Image(
        GraphicsDevice* aDevice,
        vk::Image aImage,
        vk::DeviceMemory aMemory,
        vk::ImageCreateInfo aCreateInfo,
        bool aOwning)
        : device(aDevice)
        , image(aImage)
        , memory(aMemory)
        , createInfo(aCreateInfo)
        , aspects(GetAspects(aCreateInfo.format))
        , owning(aOwning)
    {
    }

    Image::~Image()
    {
        Destroy();
    }

    void Image::Destroy()
    {
        if (owning && device)
        {
            device->DestroyImage(this);
        }
    }
}
