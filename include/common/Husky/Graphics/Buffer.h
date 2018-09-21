#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class Buffer : public GraphicsObject
    {
    public:
        Buffer(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Buffer() = 0 {};

        virtual const BufferCreateInfo& GetCreateInfo() const = 0;
    }
}

