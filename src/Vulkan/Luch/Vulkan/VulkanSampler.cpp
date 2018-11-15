#include <Luch/Vulkan/VulkanSampler.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
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
