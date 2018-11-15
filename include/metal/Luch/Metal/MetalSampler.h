#pragma once

#include <Luch/Graphics/Sampler.h>
#include <Luch/Graphics/SamplerCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
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
