#include <Luch/Vulkan/Pipeline/VulkanPipeline.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanPipeline::VulkanPipeline(
        VulkanGraphicsDevice* aDevice,
        vk::Pipeline aPipeline)
        : device(aDevice)
        , pipeline(aPipeline)
    {
    }

    VulkanPipeline::~VulkanPipeline()
    {
        Destroy();
    }

    void VulkanPipeline::Destroy()
    {
        if (device)
        {
            device->DestroyPipeline(this);
        }
    }
}
