#include <Husky/Metal/MetalSampler.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalSampler::MetalSampler(
        MetalGraphicsDevice* device,
        const SamplerCreateInfo& aCreateInfo,
        mtlpp::SamplerState aSampler)
        : Sampler(device)
        , createInfo(aCreateInfo)
        , sampler(aSampler)
    {
    }
}

