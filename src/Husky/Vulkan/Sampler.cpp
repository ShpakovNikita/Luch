#include <Husky/Vulkan/Sampler.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Sampler::Sampler(GraphicsDevice* aDevice, vk::Sampler aSampler)
        : device(aDevice)
        , sampler(aSampler)
    {
    }

    Sampler::~Sampler()
    {
        Destroy();
    }

    void Sampler::Destroy()
    {
        if (device)
        {
            device->DestroySampler(this);
        }
    }
}
