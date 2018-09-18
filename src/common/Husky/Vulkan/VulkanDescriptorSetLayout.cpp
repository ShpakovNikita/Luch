#include <Husky/Vulkan/VulkanDescriptorSetLayout.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>
#include <Husky/Vulkan/ShaderStage.h>

namespace Husky::Vulkan
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
