#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanSampler : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSampler(VulkanGraphicsDevice* device, vk::Sampler sampler);
        ~VulkanSampler();

        inline vk::Sampler GetSampler() { return sampler; }
        inline VulkanGraphicsDevice* GetDevice() const { return device; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Sampler sampler;
    };
}
