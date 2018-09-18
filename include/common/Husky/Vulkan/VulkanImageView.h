#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
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
