#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Flags.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    enum class ImageAspects
    {
        None = 0,
        Color = 1 << 0,
        Depth = 1 << 1,
        Stencil = 1 << 2,
        DepthStencil = Depth | Stencil
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(ImageAspects);

    ImageAspects GetAspects(vk::Format format);

    class Image
    {
        friend class GraphicsDevice;
    public:
        Image() = default;

        Image(Image&& other);
        Image& operator=(Image&& other);

        ~Image();

        inline vk::Image GetImage() { return image; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
        inline ImageAspects GetImageAspects() const { return aspects; }
    private:
        Image(GraphicsDevice* device, vk::Image image, vk::DeviceMemory memory, vk::ImageCreateInfo createInfo);

        GraphicsDevice* device = nullptr;
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageCreateInfo createInfo;
        ImageAspects aspects = ImageAspects::None;
    };
}
