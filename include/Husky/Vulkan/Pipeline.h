#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Pipeline : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Pipeline(GraphicsDevice* device, vk::Pipeline pipeline);

        Pipeline(const Pipeline& other) = delete;
        Pipeline(Pipeline&& other) = delete;
        Pipeline& operator=(const Pipeline& other) = delete;
        Pipeline& operator=(Pipeline&& other) = delete;

        ~Pipeline() override;

        vk::Pipeline GetPipeline() { return pipeline; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Pipeline pipeline;
    };
}
