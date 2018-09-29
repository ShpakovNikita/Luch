#include <Husky/Vulkan/VulkanPipelineCache.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
{
    VulkanPipelineCache::VulkanPipelineCache(
        VulkanGraphicsDevice* aDevice,
        vk::PipelineCache aPipelineCache)
        : device(aDevice)
        , pipelineCache(aPipelineCache)
    {
    }

    VulkanPipelineCache::~VulkanPipelineCache()
    {
        Destroy();
    }

    void VulkanPipelineCache::Destroy()
    {
        if (device)
        {
            device->DestroyPipelineCache(this);
        }
    }
}
