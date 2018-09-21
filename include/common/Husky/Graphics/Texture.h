#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class Texture : public GraphicsObject
    {
    public:
        Texture(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Texture() = 0 {};

        virtual const TextureCreateInfo& GetCreateInfo() const = 0;
    }
}
