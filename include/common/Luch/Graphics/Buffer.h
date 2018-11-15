#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class Buffer : public GraphicsObject
    {
    public:
        Buffer(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Buffer() = 0;

        virtual const BufferCreateInfo& GetCreateInfo() const = 0;

        virtual void* GetMappedMemory() = 0;
        virtual GraphicsResultValue<void*> MapMemory(int32 size, int32 offset) = 0;
        virtual GraphicsResult UnmapMemory() = 0;
//        virtual GraphicsResult FlushMemory() = 0;
//        virtual GraphicsResult InvalidateMemory() = 0;
    };

    inline Buffer::~Buffer() {}
}

