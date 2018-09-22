#pragma once

#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
{
    class Sampler : public GraphicsObject
    {
    public:
        Sampler(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Sampler() = 0 {}

        virtual const SamplerCreateInfo& GetCreateInfo() const = 0;
    }
}
