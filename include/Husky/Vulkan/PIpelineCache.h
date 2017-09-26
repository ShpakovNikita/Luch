#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class PipelineCache
    {
        friend class GraphicsDevice;
    public:
        PipelineCache() = default;
        PipelineCache(PipelineCache&& other);
        PipelineCache& operator=(PipelineCache&& other);
        ~PipelineCache();

        vk::PipelineCache GetPipelineCache() { return pipelineCache; }
    private:
        PipelineCache(GraphicsDevice* device, vk::PipelineCache pipelineCache);
        GraphicsDevice* device;
        vk::PipelineCache pipelineCache;
    };
}