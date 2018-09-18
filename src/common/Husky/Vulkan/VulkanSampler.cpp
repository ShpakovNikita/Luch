#include <Husky/Vulkan/VulkanSampler.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
{
    VulkanSampler::VulkanSampler(
        VulkanGraphicsDevice* aDevice,
        vk::Sampler aSampler)
        : device(aDevice)
        , sampler(aSampler)
    {
    }

    VulkanSampler::~VulkanSampler()
    {
        Destroy();
    }

    void VulkanSampler::Destroy()
    {
        if (device)
        {
            device->DestroySampler(this);
        }
    }
}
