#include <Husky/Metal/MetalDescriptorSet.h>
#include <Husky/Metal/MetalDescriptorSetLayout.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalTexture.h>
#include <Husky/Metal/MetalBuffer.h>
#include <Husky/Metal/MetalSampler.h>

namespace Husky::Metal
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

    void MetalDescriptorSet::WriteTexture(DescriptorSetBinding* binding, Texture* texture)
    {
        HUSKY_ASSERT(binding != nullptr);
        HUSKY_ASSERT(texture != nullptr);

        auto index = binding->GetIndex();
        if(index >= textures.size())
        {
            textures.resize(index + 1);
        }

        auto mtlTexture = static_cast<MetalTexture*>(texture);
        textures[index] = mtlTexture->texture;
    }

    void MetalDescriptorSet::WriteUniformBuffer(DescriptorSetBinding* binding, Buffer* buffer)
    {
        HUSKY_ASSERT(binding != nullptr);
        HUSKY_ASSERT(buffer != nullptr);

        auto index = binding->GetIndex();
        if(index >= buffers.size())
        {
            buffers.resize(index + 1);
            bufferOffsets.resize(index + 1);
        }

        auto mtlBuffer = static_cast<MetalBuffer*>(buffer);
        buffers[index] = mtlBuffer->buffer;
        bufferOffsets[index] = 0;
    }

    void MetalDescriptorSet::WriteSampler(DescriptorSetBinding* binding, Sampler* sampler)
    {
        HUSKY_ASSERT(binding != nullptr);
        HUSKY_ASSERT(sampler != nullptr);

        auto index = binding->GetIndex();
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
