#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
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
