#pragma once

#include <Luch/BaseObject.h>
#include <Luch/ArrayProxy.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Graphics/DescriptorSet.h>

using namespace Luch::Graphics;
namespace Luch::Vulkan
{
    class VulkanDescriptorPool : public DescriptorPool
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDescriptorPool(
            VulkanGraphicsDevice* device,
            vk::DescriptorPool descriptorPool);
        ~VulkanDescriptorPool() override;

        inline vk::DescriptorPool GetDescriptorPool() { return descriptorPool; }

        GraphicsResultRefPtr<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout* layout) override;

        vk::Result Reset();
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
    };
}
