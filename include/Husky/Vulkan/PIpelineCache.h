#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class PipelineCache : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        PipelineCache(GraphicsDevice* device, vk::PipelineCache pipelineCache);

        PipelineCache(const PipelineCache& other) = delete;
        PipelineCache(PipelineCache&& other) = delete;
        PipelineCache& operator=(const PipelineCache& other) = delete;
        PipelineCache& operator=(PipelineCache&& other) = delete;

        ~PipelineCache() override;

        vk::PipelineCache GetPipelineCache() { return pipelineCache; }
    private:
        void Destroy();

        GraphicsDevice* device;
        vk::PipelineCache pipelineCache;
    };
}
