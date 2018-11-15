#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class Sampler : public GraphicsObject
    {
    public:
        Sampler(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Sampler() = 0;

        virtual const SamplerCreateInfo& GetCreateInfo() const = 0;
    };

    inline Sampler::~Sampler() {}
}
