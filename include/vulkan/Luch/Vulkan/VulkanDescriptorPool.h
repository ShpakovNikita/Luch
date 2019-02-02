#pragma once

#include <Luch/BaseObject.h>
#include <Luch/ArrayProxy.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
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

        GraphicsResultValue<RefPtrVector<VulkanDescriptorSet>> AllocateDescriptorSets(
            const Vector<VulkanDescriptorSetLayout*>& layouts);

        GraphicsResultRefPtr<VulkanDescriptorSet> AllocateDescriptorSet(VulkanDescriptorSetLayout* layout);

        vk::Result Reset();
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
    };
}
