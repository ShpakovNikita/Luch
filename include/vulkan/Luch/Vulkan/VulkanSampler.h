#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Graphics/Sampler.h>
#include <Luch/Graphics/SamplerCreateInfo.h>

using namespace Luch::Graphics;
namespace Luch::Vulkan
{
    class VulkanSampler : public Sampler
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSampler(VulkanGraphicsDevice* device, vk::Sampler sampler);
        ~VulkanSampler() override;

        const SamplerCreateInfo& GetCreateInfo() const override { return createInfo; }
        inline vk::Sampler GetSampler() { return sampler; }
        inline VulkanGraphicsDevice* GetDevice() const { return device; }
    private:
        void Destroy();

        SamplerCreateInfo createInfo;
        VulkanGraphicsDevice* device = nullptr;
        vk::Sampler sampler;
    };
}
