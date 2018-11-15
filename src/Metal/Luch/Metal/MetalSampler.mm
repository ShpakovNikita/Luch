#include <Luch/Metal/MetalSampler.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
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

