#prama once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
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
}