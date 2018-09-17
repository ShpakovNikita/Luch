#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    PipelineLayout::PipelineLayout(GraphicsDevice* aDevice, vk::PipelineLayout aPipelineLayout)
        : device(aDevice)
        , pipelineLayout(aPipelineLayout)
    {
    }

    PipelineLayout::~PipelineLayout()
    {
        Destroy();
    }

    void PipelineLayout::Destroy()
    {
        if (device)
        {
            device->DestroyPipelineLayout(this);
        }
    }
}
