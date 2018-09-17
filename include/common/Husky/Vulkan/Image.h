#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Format.h>
#include <Husky/ImageAspects.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan/Format.h>

namespace Husky::Vulkan
{
    class Image : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Image(
            GraphicsDevice* device,
            vk::Image image,
            vk::DeviceMemory memory,
            vk::ImageCreateInfo createInfo,
            vk::MemoryRequirements memoryRequirements,
            bool owning = true);

        ~Image() override;

        inline GraphicsDevice* GetDevice() { return device; }
        inline vk::Image GetImage() { return image; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
        inline ImageAspects GetImageAspects() const { return aspects; }
        inline Format GetFormat() const { return FromVulkanFormat(createInfo.format); }
        inline const vk::MemoryRequirements& GetMemoryRequirements() const { return memoryRequirements; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageCreateInfo createInfo;
        vk::MemoryRequirements memoryRequirements;
        ImageAspects aspects = ImageAspects::None;
        bool owning = true;
    };
}
