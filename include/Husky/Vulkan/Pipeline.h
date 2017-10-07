#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Pipeline
    {
        friend class GraphicsDevice;
    public:
        Pipeline() = default;

        Pipeline(Pipeline&& other);
        Pipeline& operator=(Pipeline&& other);

        ~Pipeline();

        vk::Pipeline GetPipeline() { return pipeline; }
    private:
        Pipeline(GraphicsDevice* device, vk::Pipeline pipeline);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Pipeline pipeline;
    };
}
