#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
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
