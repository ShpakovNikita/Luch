#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    struct ImageDescriptorInfo
    {
        // ImageDescriptorInfo() = default;
        // 
        // ImageDescriptorInfo(
        //     ImageView* aImageView,
        //     vk::ImageLayout aLayout)
        //     : imageView(aImageView)
        //     , layout(aLayout)
        // {
        // }

        ImageView* imageView;
        vk::ImageLayout layout;
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

        DescriptorSetWrites& WriteImageDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<ImageDescriptorInfo>& images);

        DescriptorSetWrites& WriteSamplerDescriptors(
            DescriptorSet* descriptorSet,
            DescriptorSetBinding* binding,
            const Vector<Sampler*>& samplers);

    private:
        GraphicsDevice* device = nullptr;
        Vector<vk::WriteDescriptorSet> writes;
        Vector<Vector<vk::DescriptorBufferInfo>> bufferInfos;
        Vector<Vector<vk::DescriptorImageInfo>> imageInfos;
    };
}