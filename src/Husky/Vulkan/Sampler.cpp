#include <Husky/Vulkan/Sampler.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Sampler::Sampler(Sampler&& other)
        : device(other.device)
        , sampler(other.sampler)
    {
        other.device = nullptr;
        other.sampler = nullptr;
    }

    Sampler& Sampler::operator=(Sampler&& other)
    {
        Destroy();

        device = other.device;
        sampler = other.sampler;

        other.device = nullptr;
        other.sampler = nullptr;

        return *this;
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
