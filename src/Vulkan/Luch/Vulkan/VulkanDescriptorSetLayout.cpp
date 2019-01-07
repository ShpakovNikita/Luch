#include <Luch/Vulkan/VulkanDescriptorSetLayout.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanShaderStage.h>

namespace Luch::Vulkan
{
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
        VulkanGraphicsDevice* aDevice,
        vk::DescriptorSetLayout aDescriptorSetLayout)
        : device(aDevice)
        , descriptorSetLayout(aDescriptorSetLayout)
    {
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        Destroy();
    }

    void VulkanDescriptorSetLayout::Destroy()
    {
        if (device)
        {
            device->DestroyDescriptorSetLayout(this);
        }
    }
}
