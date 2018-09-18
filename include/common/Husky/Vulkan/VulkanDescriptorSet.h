#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
{
    class VulkanDescriptorSet : public BaseObject
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanDescriptorPool;
    public:
        VulkanDescriptorSet(VulkanGraphicsDevice* device, vk::DescriptorSet descriptorSet);

        vk::Result Free();

        static void Update(const VulkanDescriptorSetWrites& writes);

        inline vk::DescriptorSet GetDescriptorSet() { return descriptorSet; }
    private:
        VulkanGraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
    };
}
