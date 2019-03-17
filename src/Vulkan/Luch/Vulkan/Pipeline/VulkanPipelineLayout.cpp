#include <Luch/Vulkan/Pipeline/VulkanPipelineLayout.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanPipelineLayout::VulkanPipelineLayout(
        VulkanGraphicsDevice* aDevice,
        vk::PipelineLayout avkPipelineLayout)
        : PipelineLayout (aDevice)
        , device(aDevice)
        , vkPipelineLayout(avkPipelineLayout)
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
