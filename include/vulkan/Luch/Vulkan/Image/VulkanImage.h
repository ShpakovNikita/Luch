#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/ImageAspects.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Vulkan/Common/VulkanFormat.h>

namespace Luch::Vulkan
{
    class VulkanImage : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanImage(
            VulkanGraphicsDevice* device,
            vk::Image image,
            vk::DeviceMemory memory,
            vk::ImageCreateInfo createInfo,
            vk::MemoryRequirements memoryRequirements,
            bool owning = true);

        ~VulkanImage() override;

        inline VulkanGraphicsDevice* GetDevice() { return device; }
        inline vk::Image GetImage() { return image; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
        inline ImageAspects GetImageAspects() const { return aspects; }
        inline Format GetFormat() const { return FromVulkanFormat(createInfo.format); }
        inline const vk::MemoryRequirements& GetMemoryRequirements() const { return memoryRequirements; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageCreateInfo createInfo;
        vk::MemoryRequirements memoryRequirements;
        ImageAspects aspects = ImageAspects::None;
        bool owning = true;
    };
}
