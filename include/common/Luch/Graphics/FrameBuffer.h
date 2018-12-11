#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class FrameBuffer : public GraphicsObject
    {
    public:
        FrameBuffer(GraphicsDevice* device) : GraphicsObject(device) {}

        virtual const FrameBufferCreateInfo& GetCreateInfo() const = 0;
        virtual ~FrameBuffer() = 0;
    };

    inline FrameBuffer::~FrameBuffer() {}
}
