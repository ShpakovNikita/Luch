#include <Husky/Vulkan/DescriptorSetWrites.h>
#include <Husky/Vulkan/DeviceBuffer.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/Sampler.h>
#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/DescriptorSet.h>
#include <Husky/Vulkan/DescriptorSetBinding.h>

namespace Husky::Vulkan
{
    DescriptorSetWrites& DescriptorSetWrites::WriteUniformBufferDescriptors(
        DescriptorSet* descriptorSet,
        DescriptorSetBinding* binding,
        const Vector<DeviceBuffer*>& buffers)
    {
        if (buffers.empty())
        {
            return *this;
        }

        if (device == nullptr)
        {
            device = buffers.front()->GetDevice();
        }

        auto& descriptorWrite = writes.emplace_back();
        descriptorWrite.setDstSet(descriptorSet->GetDescriptorSet());
        descriptorWrite.setDstBinding(binding->GetBinding());
        descriptorWrite.setDstArrayElement(0);
        descriptorWrite.setDescriptorCount(buffers.size());
        descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        
        auto& infos = bufferInfos.emplace_back();

        for (auto& buffer : buffers)
        {
            HUSKY_ASSERT(buffer != nullptr);
            HUSKY_ASSERT(buffer->GetDevice() == device);
            auto& bufferInfo = infos.emplace_back();
            bufferInfo.setBuffer(buffer->GetBuffer());
            bufferInfo.setOffset(0);
            bufferInfo.setRange(VK_WHOLE_SIZE);
        }

        descriptorWrite.setPBufferInfo(infos.data());

        return *this;
    }


    // TODO get rid of copy-paste
    DescriptorSetWrites& DescriptorSetWrites::WriteStorageBufferDescriptors(
        DescriptorSet* descriptorSet,
        DescriptorSetBinding* binding,
        const Vector<DeviceBuffer*>& buffers)
    {
        if (buffers.empty())
        {
            return *this;
        }

        if (device == nullptr)
        {
            device = buffers.front()->GetDevice();
        }

        auto& descriptorWrite = writes.emplace_back();
        descriptorWrite.setDstSet(descriptorSet->GetDescriptorSet());
        descriptorWrite.setDstBinding(binding->GetBinding());
        descriptorWrite.setDstArrayElement(0);
        descriptorWrite.setDescriptorCount(buffers.size());
        descriptorWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);

        auto& infos = bufferInfos.emplace_back();

        for (auto& buffer : buffers)
        {
            HUSKY_ASSERT(buffer != nullptr);
            HUSKY_ASSERT(buffer->GetDevice() == device);
            auto& bufferInfo = infos.emplace_back();
            bufferInfo.setBuffer(buffer->GetBuffer());
            bufferInfo.setOffset(0);
            bufferInfo.setRange(VK_WHOLE_SIZE);
        }

        descriptorWrite.setPBufferInfo(infos.data());

        return *this;
    }

    DescriptorSetWrites & DescriptorSetWrites::WriteImageDescriptors(
        DescriptorSet* descriptorSet,
        DescriptorSetBinding* binding,
        const Vector<ImageDescriptorInfo>& images)
    {
        if (images.empty())
        {
            return *this;
        }

        if (device == nullptr)
        {
            device = images.front().imageView->GetDevice();
        }

        auto& descriptorWrite = writes.emplace_back();
        descriptorWrite.setDstSet(descriptorSet->GetDescriptorSet());
        descriptorWrite.setDstBinding(binding->GetBinding());
        descriptorWrite.setDstArrayElement(0);
        descriptorWrite.setDescriptorCount(images.size());
        descriptorWrite.setDescriptorType(vk::DescriptorType::eSampledImage);

        auto& infos = imageInfos.emplace_back();

        for (auto& image : images)
        {
            HUSKY_ASSERT(image.imageView != nullptr);
            HUSKY_ASSERT(image.imageView->GetDevice() == device);

            auto& imageInfo = infos.emplace_back();
            imageInfo.setImageLayout(image.layout);
            imageInfo.setImageView(image.imageView->GetImageView());
        }

        descriptorWrite.setPImageInfo(infos.data());

        return *this;
    }

    DescriptorSetWrites& DescriptorSetWrites::WriteSamplerDescriptors(
        DescriptorSet* descriptorSet,
        DescriptorSetBinding* binding,
        const Vector<Sampler*>& samplers)
    {
        if (samplers.empty())
        {
            return *this;
        }

        if (device == nullptr)
        {
            device = samplers.front()->GetDevice();
        }

        auto& descriptorWrite = writes.emplace_back();
        descriptorWrite.setDstSet(descriptorSet->GetDescriptorSet());
        descriptorWrite.setDstBinding(binding->GetBinding());
        descriptorWrite.setDstArrayElement(0);
        descriptorWrite.setDescriptorCount(samplers.size());
        descriptorWrite.setDescriptorType(vk::DescriptorType::eSampler);

        auto& infos = imageInfos.emplace_back();

        for (auto& sampler : samplers)
        {
            HUSKY_ASSERT(sampler->GetDevice() == device);

            auto& imageInfo = infos.emplace_back();
            imageInfo.setSampler(sampler->GetSampler());
        }

        descriptorWrite.setPImageInfo(infos.data());

        return *this;
    }
    DescriptorSetWrites & DescriptorSetWrites::WriteCombinedImageDescriptors(
        DescriptorSet* descriptorSet,
        DescriptorSetBinding* binding,
        const Vector<ImageDescriptorInfo>& images)
    {
        if (images.empty())
        {
            return *this;
        }

        if (device == nullptr)
        {
            device = images.front().imageView->GetDevice();
        }

        auto& descriptorWrite = writes.emplace_back();
        descriptorWrite.setDstSet(descriptorSet->GetDescriptorSet());
        descriptorWrite.setDstBinding(binding->GetBinding());
        descriptorWrite.setDstArrayElement(0);
        descriptorWrite.setDescriptorCount(images.size());
        descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

        auto& infos = imageInfos.emplace_back();

        for (auto& image : images)
        {
            HUSKY_ASSERT(image.imageView != nullptr);
            HUSKY_ASSERT(image.imageView->GetDevice() == device);
            HUSKY_ASSERT(image.sampler->GetDevice() == device);

            auto& imageInfo = infos.emplace_back();
            imageInfo.setImageLayout(image.layout);
            imageInfo.setImageView(image.imageView->GetImageView());
            imageInfo.setSampler(image.sampler->GetSampler());
        }

        descriptorWrite.setPImageInfo(infos.data());

        return *this;
    }
}
