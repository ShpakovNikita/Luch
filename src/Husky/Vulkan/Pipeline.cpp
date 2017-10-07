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

    Pipeline& Pipeline::operator=(Pipeline && other)
    {
        Destroy();

        device = other.device;
        pipeline = other.pipeline;

        other.device = nullptr;
        other.pipeline = nullptr;

        return *this;
    }

    Pipeline::~Pipeline()
    {
        Destroy();
    }

    void Pipeline::Destroy()
    {
        if (device)
        {
            device->DestroyPipeline(this);
        }
    }
}
