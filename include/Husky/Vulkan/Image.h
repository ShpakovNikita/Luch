#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/ImageAspects.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/BaseObject.h>

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
        inline Format GetFormat() const { return FromVulkanFormat(createInfo.format); }
    private:
        Image(GraphicsDevice* device, vk::Image image, vk::DeviceMemory memory, vk::ImageCreateInfo createInfo, bool owning = true);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageCreateInfo createInfo;
        ImageAspects aspects = ImageAspects::None;
        bool owning = true;
    };

    class ImageObject : public BaseObject
    {
    public:
        ImageObject(Image&& aImage)
            : image(std::move(aImage))
        {
        }

        inline Image* GetImage() { return &image; }
    private:
        Image image;
    };
}
