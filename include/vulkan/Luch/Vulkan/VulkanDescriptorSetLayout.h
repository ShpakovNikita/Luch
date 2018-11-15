#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    // TODO propagate index from pipeline layout
    class VulkanDescriptorSetLayout : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDescriptorSetLayout(
            VulkanGraphicsDevice* device,
            vk::DescriptorSetLayout descriptorSetLayout);

        ~VulkanDescriptorSetLayout() override;

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
