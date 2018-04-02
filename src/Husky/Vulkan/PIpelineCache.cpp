#include <Husky/Vulkan/PipelineCache.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    PipelineCache::PipelineCache(GraphicsDevice* aDevice, vk::PipelineCache aPipelineCache)
        : device(aDevice)
        , pipelineCache(aPipelineCache)
    {
    }

    PipelineCache::~PipelineCache()
    {
        Destroy();
    }

    void PipelineCache::Destroy()
    {
        if (device)
        {
            device->DestroyPipelineCache(this);
        }
    }
}
