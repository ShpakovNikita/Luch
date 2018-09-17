#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Pipeline::Pipeline(GraphicsDevice* aDevice, vk::Pipeline aPipeline)
        : device(aDevice)
        , pipeline(aPipeline)
    {
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
