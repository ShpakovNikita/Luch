#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSetWrites.h>
#include <Luch/Vulkan/Buffer/VulkanDeviceBuffer.h>
#include <Luch/Vulkan/Image/VulkanImage.h>
#include <Luch/Vulkan/Image/VulkanSampler.h>
#include <Luch/Vulkan/Image/VulkanImageView.h>
#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSet.h>
#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSetBinding.h>
namespace Luch::Vulkan
{
    VulkanDescriptorSetWrites& VulkanDescriptorSetWrites::WriteUniformBufferDescriptors(
        VulkanDescriptorSet* descriptorSet,
        VulkanDescriptorSetBinding* binding,
        const Vector<VulkanDeviceBuffer*>& buffers)
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
            LUCH_ASSERT(buffer != nullptr);
            LUCH_ASSERT(buffer->GetDevice() == device);
            auto& bufferInfo = infos.emplace_back();
            bufferInfo.setBuffer(buffer->GetBuffer());
            bufferInfo.setOffset(0);
            bufferInfo.setRange(VK_WHOLE_SIZE);
        }

        descriptorWrite.setPBufferInfo(infos.data());

        return *this;
    }


    // TODO get rid of copy-paste
    VulkanDescriptorSetWrites& VulkanDescriptorSetWrites::WriteStorageBufferDescriptors(
        VulkanDescriptorSet* descriptorSet,
        VulkanDescriptorSetBinding* binding,
        const Vector<VulkanDeviceBuffer*>& buffers)
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
            LUCH_ASSERT(buffer != nullptr);
            LUCH_ASSERT(buffer->GetDevice() == device);
            auto& bufferInfo = infos.emplace_back();
            bufferInfo.setBuffer(buffer->GetBuffer());
            bufferInfo.setOffset(0);
            bufferInfo.setRange(VK_WHOLE_SIZE);
        }

        descriptorWrite.setPBufferInfo(infos.data());

        return *this;
    }

    VulkanDescriptorSetWrites & VulkanDescriptorSetWrites::WriteImageDescriptors(
        VulkanDescriptorSet* descriptorSet,
        VulkanDescriptorSetBinding* binding,
        const Vector<VulkanImageDescriptorInfo>& images)
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
            LUCH_ASSERT(image.imageView != nullptr);
            LUCH_ASSERT(image.imageView->GetDevice() == device);

            auto& imageInfo = infos.emplace_back();
            imageInfo.setImageLayout(image.layout);
            imageInfo.setImageView(image.imageView->GetImageView());
        }

        descriptorWrite.setPImageInfo(infos.data());

        return *this;
    }

    VulkanDescriptorSetWrites& VulkanDescriptorSetWrites::WriteSamplerDescriptors(
        VulkanDescriptorSet* descriptorSet,
        VulkanDescriptorSetBinding* binding,
        const Vector<VulkanSampler*>& samplers)
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
            LUCH_ASSERT(sampler->GetDevice() == device);

            auto& imageInfo = infos.emplace_back();
            imageInfo.setSampler(sampler->GetSampler());
        }

        descriptorWrite.setPImageInfo(infos.data());

        return *this;
    }
    VulkanDescriptorSetWrites & VulkanDescriptorSetWrites::WriteCombinedImageDescriptors(
        VulkanDescriptorSet* descriptorSet,
        VulkanDescriptorSetBinding* binding,
        const Vector<VulkanImageDescriptorInfo>& images)
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
            LUCH_ASSERT(image.imageView != nullptr);
            LUCH_ASSERT(image.imageView->GetDevice() == device);
            LUCH_ASSERT(image.sampler->GetDevice() == device);

            auto& imageInfo = infos.emplace_back();
            imageInfo.setImageLayout(image.layout);
            imageInfo.setImageView(image.imageView->GetImageView());
            imageInfo.setSampler(image.sampler->GetSampler());
        }

        descriptorWrite.setPImageInfo(infos.data());

        return *this;
    }
}
