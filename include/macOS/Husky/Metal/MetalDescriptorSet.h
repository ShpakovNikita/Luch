#pragma once

#include <Husky/Graphics/DescriptorSet.h>
#include <Husky/Graphics/DescriptorSetType.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalDescriptorSet : public DescriptorSet
    {
        friend class MetalGraphicsCommandList;
    public:
        MetalDescriptorSet(MetalGraphicsDevice* device, DescriptorSetType type, int32 start);

        DescriptorSetType GetType() const override { return type; }
    private:
        Vector<mtlpp::Texture> textures;
        Vector<mtlpp::Buffer> buffers;
        Vector<int32> bufferOffsets;
        Vector<mtlpp::SamplerState> samplers;
        DescriptorSetType type;
        int32 start = 0;
    };
}
