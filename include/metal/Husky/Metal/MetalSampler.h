#pragma once

#include <Husky/Graphics/Sampler.h>
#include <Husky/Graphics/SamplerCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalSampler : public Sampler
    {
        friend class MetalDescriptorSet;
    public:
        MetalSampler(
            MetalGraphicsDevice* device,
            const SamplerCreateInfo& createInfo,
            mtlpp::SamplerState sampler);

        const SamplerCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        SamplerCreateInfo createInfo;
        mtlpp::SamplerState sampler;
    };
}
