#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    struct ImageDescriptorInfo
    {
        ImageView* imageView = nullptr;
        Sampler* sampler = nullptr;
        vk::ImageLayout layout = vk::ImageLayout::eGeneral;
    };

    class DescriptorSetWrites
    {
        friend class DescriptorSet;
    public:
        DescriptorSetWrites() = default;
        DescriptorSetWrites(DescriptorSetWrites&& other) = default;
        DescriptorSetWrites& operator=(DescriptorSetWrites&& other) = default;

        DescriptorSetWrites& WriteUniformBufferDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<Buffer*>& buffers);

        DescriptorSetWrites& WriteStorageBufferDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<Buffer*>& buffers);

        DescriptorSetWrites& WriteImageDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<ImageDescriptorInfo>& images);

        DescriptorSetWrites& WriteSamplerDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<Sampler*>& samplers);

        DescriptorSetWrites& WriteCombinedImageDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<ImageDescriptorInfo>& images);

    private:
        GraphicsDevice* device = nullptr;
        Vector<vk::WriteDescriptorSet> writes;
        Vector<Vector<vk::DescriptorBufferInfo>> bufferInfos;
        Vector<Vector<vk::DescriptorImageInfo>> imageInfos;
    };
}