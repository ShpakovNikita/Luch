#pragma once

#include <vulkan/vulkan.hpp>
#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
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
