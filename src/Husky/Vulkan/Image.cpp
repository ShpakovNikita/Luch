#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    ImageAspects GetAspects(vk::Format format)
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
        vk::ImageCreateInfo aCreateInfo)
        : device(aDevice)
        , image(aImage)
        , memory(aMemory)
        , createInfo(aCreateInfo)
        , aspects(GetAspects(aCreateInfo.format))
    {
    }

    Image::Image(Image&& other)
        : device(other.device)
        , image(other.image)
        , memory(other.memory)
        , createInfo(other.createInfo)
        , aspects(other.aspects)
    {
        other.device = nullptr;
        other.image = nullptr;
        other.memory = nullptr;
    }

    Image& Image::operator=(Image&& other)
    {
        Destroy();

        device = other.device;
        image = other.image;
        memory = other.memory;
        createInfo = other.createInfo;
        aspects = other.aspects;

        other.device = nullptr;
        other.image = nullptr;
        other.memory = nullptr;

        return *this;
    }

    Image::~Image()
    {
        Destroy();
    }

    void Image::Destroy()
    {
        if (device)
        {
            device->DestroyImage(this);
        }
    }
}
