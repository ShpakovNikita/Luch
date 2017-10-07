#include <Husky/Vulkan/PipelineCache.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    PipelineCache::PipelineCache(GraphicsDevice* aDevice, vk::PipelineCache aPipelineCache)
        : device(aDevice)
        , pipelineCache(aPipelineCache)
    {
    }

    PipelineCache::PipelineCache(PipelineCache&& other)
        : device(other.device)
        , pipelineCache(other.pipelineCache)
    {
        other.device = nullptr;
        other.pipelineCache = nullptr;
    }

    PipelineCache & PipelineCache::operator=(PipelineCache&& other)
    {
        Destroy();

        device = other.device;
        pipelineCache = other.pipelineCache;

        other.device = nullptr;
        other.pipelineCache = nullptr;

        return *this;
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
