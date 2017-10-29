#include <Husky/Vulkan/DescriptorSet.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>

namespace Husky::Vulkan
{
    DescriptorSet::DescriptorSet(GraphicsDevice* aDevice, vk::DescriptorSet aDescriptorSet)
        : device(aDevice)
        , descriptorSet(aDescriptorSet)
    {
    }

    DescriptorSet::DescriptorSet(DescriptorSet&& other)
        : device(other.device)
        , descriptorSet(other.descriptorSet)
    {
        other.device = nullptr;
        other.descriptorSet = nullptr;
    }

    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other)
    {
        device = other.device;
        descriptorSet = other.descriptorSet;

        other.device = nullptr;
        other.descriptorSet = nullptr;

        return *this;
    }

    inline vk::Result DescriptorSet::Free()
    {
        return device->GetDevice().freeDescriptorSets(descriptorPool, { descriptorSet });
    }

    void DescriptorSet::Update(const DescriptorSetWrites& writes)
    {
        if (writes.writes.empty())
        {
            return;
        }

        writes.device->device.updateDescriptorSets(writes.writes, {});
    }

    DescriptorSetWrites& DescriptorSetWrites::WriteUniformBufferDescriptors(DescriptorSet* descriptorSet, DescriptorSetBinding* binding, Vector<Buffer*> buffers)
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
            HUSKY_ASSERT(buffer->GetDevice() == device);
            auto& bufferInfo = infos.emplace_back();
            bufferInfo.setBuffer(buffer->GetBuffer());
            bufferInfo.setOffset(0);
            bufferInfo.setRange(VK_WHOLE_SIZE);
        }

        descriptorWrite.setPBufferInfo(infos.data());

        return *this;
    }

}
