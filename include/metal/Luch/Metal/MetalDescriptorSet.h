#pragma once

#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/DescriptorSetType.h>
#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalDescriptorSet : public DescriptorSet
    {
        friend class MetalGraphicsCommandList;
        friend class MetalComputeCommandList;
    public:
        MetalDescriptorSet(
            MetalGraphicsDevice* device,
            MetalDescriptorSetLayout* descriptorSetLayout);

        DescriptorSetLayout* GetDescriptorSetLayout() override;

        void WriteTexture(const DescriptorSetBinding& binding, Texture* texture) override;
        void WriteUniformBuffer(const DescriptorSetBinding& binding, Buffer* buffer, int32 offset) override;
        void WriteSampler(const DescriptorSetBinding& binding, Sampler* sampler) override;
        void Update() override;
    private:
        Vector<mtlpp::Texture> textures;
        Vector<mtlpp::Buffer> buffers;
        Vector<int32> bufferOffsets;
        Vector<mtlpp::SamplerState> samplers;
        MetalDescriptorSetLayout* descriptorSetLayout;
    };
}
