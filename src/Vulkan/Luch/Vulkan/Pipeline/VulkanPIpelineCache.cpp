#include <Luch/Vulkan/Pipeline/VulkanPipelineCache.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
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
