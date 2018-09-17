#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Pipeline : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Pipeline(GraphicsDevice* device, vk::Pipeline pipeline);
        ~Pipeline() override;

        vk::Pipeline GetPipeline() { return pipeline; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Pipeline pipeline;
    };
}
