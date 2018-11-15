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
        HUSKY_ASSERT(texture != nullptr);
        HUSKY_ASSERT(samplers.empty());
        HUSKY_ASSERT(buffers.empty());

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
        HUSKY_ASSERT(buffer != nullptr);
        HUSKY_ASSERT(textures.empty());
        HUSKY_ASSERT(samplers.empty());

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

    void MetalDescriptorSet::WriteSampler(const DescriptorSetBinding& binding, Sampler* sampler)
    {
        HUSKY_ASSERT(sampler != nullptr);
        HUSKY_ASSERT(textures.empty());
        HUSKY_ASSERT(buffers.empty());

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
