#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
{
    class VulkanSampler : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSampler(VulkanGraphicsDevice* device, vk::Sampler sampler);
        ~Sampler();

        inline vk::Sampler GetSampler() { return sampler; }
        inline VulkanGraphicsDevice* GetDevice() const { return device; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Sampler sampler;
    };
}
