#pragma once

#include <Husky/BaseObject.h>
#include <Husky/ArrayProxy.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
{
    class VulkanDescriptorPool : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDescriptorPool(
            VulkanGraphicsDevice* device,
            vk::DescriptorPool descriptorPool);
        ~VulkanDescriptorPool() override;

        inline vk::DescriptorPool GetDescriptorPool() { return descriptorPool; }

        VulkanResultValue<RefPtrVector<VulkanDescriptorSet>> AllocateDescriptorSets(
            const Vector<VulkanDescriptorSetLayout*>& layouts);

        VulkanRefResultValue<VulkanDescriptorSet> AllocateDescriptorSet(VulkanDescriptorSetLayout* layout);

        vk::Result Reset();
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
    };
}
