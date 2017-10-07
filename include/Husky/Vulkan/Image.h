#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/ImageAspects.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

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
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageCreateInfo createInfo;
        ImageAspects aspects = ImageAspects::None;
    };
}
