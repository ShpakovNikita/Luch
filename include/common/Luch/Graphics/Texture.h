#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class Texture : public GraphicsObject
    {
    public:
        Texture(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Texture() = 0;

        virtual const TextureCreateInfo& GetCreateInfo() const = 0;
    };

    inline Texture::~Texture() {}
}
