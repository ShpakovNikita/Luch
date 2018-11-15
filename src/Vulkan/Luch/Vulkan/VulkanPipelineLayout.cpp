#include <Luch/Vulkan/VulkanPipelineLayout.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
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
