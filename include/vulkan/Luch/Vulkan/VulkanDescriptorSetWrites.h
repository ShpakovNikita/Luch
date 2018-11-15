#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
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
            const Vector<DeviceBuffer*>& buffers);

        DescriptorSetWrites& WriteStorageBufferDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<DeviceBuffer*>& buffers);

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
