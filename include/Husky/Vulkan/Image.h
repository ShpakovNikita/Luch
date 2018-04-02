#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/ImageAspects.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/BaseObject.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Image : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Image(GraphicsDevice* device, vk::Image image, vk::DeviceMemory memory, vk::ImageCreateInfo createInfo, bool owning = true);

        Image(const Image& other) = delete;
        Image(Image&& other) = delete;
        Image& operator=(const Image& other) = delete;
        Image& operator=(Image&& other) = delete;

        ~Image() override;

        inline vk::Image GetImage() { return image; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
        inline ImageAspects GetImageAspects() const { return aspects; }
        inline Format GetFormat() const { return FromVulkanFormat(createInfo.format); }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageCreateInfo createInfo;
        ImageAspects aspects = ImageAspects::None;
        bool owning = true;
    };
}
