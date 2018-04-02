#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Sampler : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Sampler(GraphicsDevice* device, vk::Sampler sampler);

        Sampler(Sampler&& other) = delete;
        Sampler(const Sampler& other) = delete;
        Sampler& operator=(const Sampler& other) = delete;
        Sampler& operator=(Sampler&& other) = delete;

        ~Sampler();

        inline vk::Sampler GetSampler() { return sampler; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Sampler sampler;
    };
}
