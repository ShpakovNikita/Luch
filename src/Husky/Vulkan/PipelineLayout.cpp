#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    PipelineLayout::PipelineLayout(GraphicsDevice* aDevice, vk::PipelineLayout aPipelineLayout)
        : device(aDevice)
        , pipelineLayout(aPipelineLayout)
    {
    }

    PipelineLayout::PipelineLayout(PipelineLayout&& other)
        : device(other.device)
        , pipelineLayout(other.pipelineLayout)
    {
        other.device = nullptr;
        other.pipelineLayout = nullptr;
    }

    PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other)
    {
        device = other.device;
        pipelineLayout = other.pipelineLayout;

        other.device = nullptr;
        other.pipelineLayout = nullptr;

        return *this;
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