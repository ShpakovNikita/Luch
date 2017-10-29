#pragma once

#include <Husky/Vulkan.h>
#include <Husky/ArrayProxy.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class DescriptorSetBinding;
    class Buffer;

    class DescriptorSetWrites
    {
        friend class DescriptorSet;
    public:
        DescriptorSetWrites() = default;
        DescriptorSetWrites(DescriptorSetWrites&& other) = default;
        DescriptorSetWrites& operator=(DescriptorSetWrites&& other) = default;

        DescriptorSetWrites& WriteUniformBufferDescriptors(DescriptorSet* descriptorSet, DescriptorSetBinding* binding, Vector<Buffer*> buffers);
    private:
        GraphicsDevice* device = nullptr;
        Vector<vk::WriteDescriptorSet> writes;
        Vector<Vector<vk::DescriptorBufferInfo>> bufferInfos;
    };

    class DescriptorSet
    {
        friend class GraphicsDevice;
        friend class DescriptorPool;
    public:
        DescriptorSet() = default;
        DescriptorSet(GraphicsDevice* device, vk::DescriptorSet descriptorSet);

        DescriptorSet(DescriptorSet&& other);
        DescriptorSet& operator=(DescriptorSet&& other);

        vk::Result Free();

        static void Update(const DescriptorSetWrites& writes);

        inline vk::DescriptorSet GetDescriptorSet() { return descriptorSet; }
    private:
        GraphicsDevice* device = nullptr;
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
    };
}
