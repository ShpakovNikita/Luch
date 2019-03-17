#include <Luch/Metal/MetalDescriptorSet.h>
#include <Luch/Metal/MetalDescriptorSetLayout.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalTexture.h>
#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalSampler.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalDescriptorSet::MetalDescriptorSet(
        MetalGraphicsDevice* device,
        MetalDescriptorSetLayout* aDescriptorSetLayout)
        : DescriptorSet(device)
        , descriptorSetLayout(aDescriptorSetLayout)
    {
    }

    DescriptorSetLayout* MetalDescriptorSet::GetDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }

    // TODO Resize based on descriptor set layout

    void MetalDescriptorSet::WriteTexture(const DescriptorSetBinding& binding, Texture* texture)
    {
        LUCH_ASSERT(texture != nullptr);
        LUCH_ASSERT(samplers.empty());
        LUCH_ASSERT(buffers.empty());
        LUCH_ASSERT(binding.GetType() == ResourceType::Texture);

        auto index = binding.GetIndex();
        if(index >= textures.size())
        {
            textures.resize(index + 1);
        }

        auto mtlTexture = static_cast<MetalTexture*>(texture);
        textures[index] = mtlTexture->texture;
    }

    void MetalDescriptorSet::WriteUniformBuffer(const DescriptorSetBinding& binding, Buffer* buffer, int32 offset)
    {
        LUCH_ASSERT(buffer != nullptr);
        LUCH_ASSERT(textures.empty());
        LUCH_ASSERT(samplers.empty());
        LUCH_ASSERT(binding.GetType() == ResourceType::UniformBuffer);

        auto index = binding.GetIndex();
        if(index >= buffers.size())
        {
            buffers.resize(index + 1);
            bufferOffsets.resize(index + 1);
        }

        auto mtlBuffer = static_cast<MetalBuffer*>(buffer);
        buffers[index] = mtlBuffer->buffer;
        bufferOffsets[index] = offset;
    }

    void MetalDescriptorSet::WriteThreadgroupMemory(const DescriptorSetBinding& binding, int32 length, int32 offset)
    {
        LUCH_ASSERT(buffers.empty());
        LUCH_ASSERT(textures.empty());
        LUCH_ASSERT(samplers.empty());
        LUCH_ASSERT(binding.GetType() == ResourceType::ThreadgroupMemory);

        auto index = binding.GetIndex();
        if(index >= memoryLengths.size())
        {
            memoryLengths.resize(index + 1);
            bufferOffsets.resize(index + 1);
        }

        memoryLengths[index] = length;
        bufferOffsets[index] = offset;
    }

    void MetalDescriptorSet::WriteSampler(const DescriptorSetBinding& binding, Sampler* sampler)
    {
        LUCH_ASSERT(sampler != nullptr);
        LUCH_ASSERT(textures.empty());
        LUCH_ASSERT(buffers.empty());
        LUCH_ASSERT(binding.GetType() == ResourceType::Sampler);

        auto index = binding.GetIndex();
        if(index >= samplers.size())
        {
            samplers.resize(index + 1);
        }

        auto mtlSampler = static_cast<MetalSampler*>(sampler);
        samplers[index] = mtlSampler->sampler;
    }

    void MetalDescriptorSet::Update()
    {
    }
}
