#include <Husky/Vulkan/VulkanPipeline.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
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
