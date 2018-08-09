#pragma once

#include <Husky/BaseObject.h>
#include <Husky/ArrayProxy.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class DescriptorPool : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        DescriptorPool(GraphicsDevice* device, vk::DescriptorPool descriptorPool);
        ~DescriptorPool() override;

        inline vk::DescriptorPool GetDescriptorPool() { return descriptorPool; }

        VulkanResultValue<RefPtrVector<DescriptorSet>> AllocateDescriptorSets(const Vector<DescriptorSetLayout*>& layouts);
        VulkanRefResultValue<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout* layout);

        vk::Result Reset();
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
    };
}
