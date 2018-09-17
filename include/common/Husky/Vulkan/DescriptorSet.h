#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class DescriptorSet : public BaseObject
    {
        friend class GraphicsDevice;
        friend class DescriptorPool;
    public:
        DescriptorSet(GraphicsDevice* device, vk::DescriptorSet descriptorSet);

        vk::Result Free();

        static void Update(const DescriptorSetWrites& writes);

        inline vk::DescriptorSet GetDescriptorSet() { return descriptorSet; }
    private:
        GraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
    };
}
