#pragma once

#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    struct VulkanImageDescriptorInfo
    {
        VulkanImageView* imageView = nullptr;
        VulkanSampler* sampler = nullptr;
        vk::ImageLayout layout = vk::ImageLayout::eGeneral;
    };

    class VulkanDescriptorSetWrites
    {
        friend class VulkanDescriptorSet;
    public:
        VulkanDescriptorSetWrites() = default;
        VulkanDescriptorSetWrites(VulkanDescriptorSetWrites&& other) = default;
        VulkanDescriptorSetWrites& operator=(VulkanDescriptorSetWrites&& other) = default;

        VulkanDescriptorSetWrites& WriteUniformBufferDescriptors(
            VulkanDescriptorSet* descriptorSet,
            VulkanDescriptorSetBinding* binding,
            const Vector<VulkanDeviceBuffer*>& buffers);

        VulkanDescriptorSetWrites& WriteStorageBufferDescriptors(
            VulkanDescriptorSet* descriptorSet,
            VulkanDescriptorSetBinding* binding,
            const Vector<VulkanDeviceBuffer*>& buffers);

        VulkanDescriptorSetWrites& WriteImageDescriptors(
            VulkanDescriptorSet* descriptorSet,
            VulkanDescriptorSetBinding* binding,
            const Vector<VulkanImageDescriptorInfo>& images);

        VulkanDescriptorSetWrites& WriteSamplerDescriptors(
            VulkanDescriptorSet* descriptorSet,
            VulkanDescriptorSetBinding* binding,
            const Vector<VulkanSampler*>& samplers);

        VulkanDescriptorSetWrites& WriteCombinedImageDescriptors(
            VulkanDescriptorSet* descriptorSet,
            VulkanDescriptorSetBinding* binding,
            const Vector<VulkanImageDescriptorInfo>& images);

    private:
        VulkanGraphicsDevice* device = nullptr;
        Vector<vk::WriteDescriptorSet> writes;
        Vector<Vector<vk::DescriptorBufferInfo>> bufferInfos;
        Vector<Vector<vk::DescriptorImageInfo>> imageInfos;
    };
}
