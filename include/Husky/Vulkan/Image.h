#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class Image
    {
    public:
        Image()
        {
        }

        Image(const Image& other) = delete;

        Image(Image&& other)
            : allocationCallbacks(other.allocationCallbacks)
            , createInfo(other.createInfo)
            , device(other.device)
            , image(other.image)
            , imageView(other.imageView)
            , deviceMemory(other.deviceMemory)
        {
            other.device = nullptr;
            other.image = nullptr;
            other.imageView = nullptr;
            other.deviceMemory = nullptr;
        }

        ~Image()
        {
            if (device)
            {
                device.destroyImageView(imageView, allocationCallbacks);
                device.destroyImage(image, allocationCallbacks);
                device.freeMemory(deviceMemory, allocationCallbacks);
            }
        }
    private:
        vk::AllocationCallbacks allocationCallbacks;
        vk::ImageCreateInfo createInfo;
        vk::Device device;
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory deviceMemory;
    };
}