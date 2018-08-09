#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class Sampler : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Sampler(GraphicsDevice* device, vk::Sampler sampler);
        ~Sampler();

        inline vk::Sampler GetSampler() { return sampler; }
        inline GraphicsDevice* GetDevice() const { return device; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Sampler sampler;
    };
}
