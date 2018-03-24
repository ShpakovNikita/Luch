#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Sampler
    {
        friend class GraphicsDevice;
    public:
        Sampler() = default;

        Sampler(Sampler&& other);
        Sampler& operator=(Sampler&& other);

        ~Sampler();

        inline vk::Sampler GetSampler() { return sampler; }
    private:
        Sampler(GraphicsDevice* device, vk::Sampler sampler);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Sampler sampler;
    };

    class SamplerObject : public BaseObject
    {
    public:
        inline explicit SamplerObject(Sampler aSampler)
            : sampler(std::move(aSampler))
        {
        }

        inline Sampler* GetSampler() { return &sampler; }
    private:
        Sampler sampler;
    };
}
