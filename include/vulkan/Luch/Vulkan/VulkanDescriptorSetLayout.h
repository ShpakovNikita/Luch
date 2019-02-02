#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
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
            vk::DescriptorSetLayout descriptorSetLayout);

        ~VulkanDescriptorSetLayout() override;

        const Graphics::DescriptorSetLayoutCreateInfo& GetCreateInfo() const override
        {
            return createInfoUnused;
        }

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        void Destroy();

        Graphics::DescriptorSetLayoutCreateInfo createInfoUnused; // todo: fix
        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
