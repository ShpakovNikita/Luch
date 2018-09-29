#include <Husky/Vulkan/VulkanPipelineLayout.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
{
    VulkanPipelineLayout::VulkanPipelineLayout(
        VulkanGraphicsDevice* aDevice,
        vk::PipelineLayout aPipelineLayout)
        : device(aDevice)
        , pipelineLayout(aPipelineLayout)
    {
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        Destroy();
    }

    void VulkanPipelineLayout::Destroy()
    {
        if (device)
        {
            device->DestroyPipelineLayout(this);
        }
    }
}
