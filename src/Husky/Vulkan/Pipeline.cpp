#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Pipeline::Pipeline(GraphicsDevice* aDevice, vk::Pipeline aPipeline)
        : device(aDevice)
        , pipeline(aPipeline)
    {
    }

    Pipeline::Pipeline(Pipeline&& other)
        : device(other.device)
        , pipeline(other.pipeline)
    {
        other.device = nullptr;
        other.pipeline = nullptr;
    }

    Pipeline::~Pipeline()
    {
        if (device)
        {
            device->DestroyPipeline(this);
        }
    }

    Pipeline& Pipeline::operator=(Pipeline && other)
    {
        device = other.device;
        pipeline = other.pipeline;

        other.device = nullptr;
        other.pipeline = nullptr;

        return *this;
    }
}