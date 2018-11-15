#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanImageView : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        VulkanImageView(VulkanGraphicsDevice* device, vk::ImageView imageView);
        ~VulkanImageView();

        inline VulkanGraphicsDevice* GetDevice() { return device; }
        inline vk::ImageView GetImageView() { return imageView; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::ImageView imageView;
    };
}
