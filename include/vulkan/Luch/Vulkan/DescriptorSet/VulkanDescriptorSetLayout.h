#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    // TODO propagate index from pipeline layout
    class VulkanDescriptorSetLayout : public DescriptorSetLayout
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDescriptorSetLayout(
            VulkanGraphicsDevice* device,
            const DescriptorSetLayoutCreateInfo& ci,
            vk::DescriptorSetLayout descriptorSetLayout);

        ~VulkanDescriptorSetLayout() override;

        const DescriptorSetLayoutCreateInfo& GetCreateInfo() const override
        {
            return createInfo;
        }

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        void Destroy();

        DescriptorSetLayoutCreateInfo createInfo;
        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
