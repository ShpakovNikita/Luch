#pragma once

#include <Husky/Vulkan.h>
#include <Husky/ArrayProxy.h>
#include <Husky/Vulkan/DescriptorSet.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class DescriptorPool
    {
        friend class GraphicsDevice;
    public:
        DescriptorPool() = default;

        ~DescriptorPool();

        DescriptorPool(DescriptorPool&& other);
        DescriptorPool& operator=(DescriptorPool&& other);

        inline vk::DescriptorPool GetDescriptorPool() { return descriptorPool; }

        VulkanResultValue<Vector<DescriptorSet>> AllocateDescriptorSets(Vector<DescriptorSetLayout*> layouts);
        VulkanResultValue<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout* layout);

        vk::Result Reset();
    private:
        DescriptorPool(GraphicsDevice* device, vk::DescriptorPool descriptorPool);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
    };
}
