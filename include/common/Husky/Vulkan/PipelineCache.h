#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class PipelineCache : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        PipelineCache(GraphicsDevice* device, vk::PipelineCache pipelineCache);
        ~PipelineCache() override;

        vk::PipelineCache GetPipelineCache() { return pipelineCache; }
    private:
        void Destroy();

        GraphicsDevice* device;
        vk::PipelineCache pipelineCache;
    };
}
