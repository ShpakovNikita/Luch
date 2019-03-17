#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class Semaphore : public GraphicsObject
    {
    public:
        Semaphore(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual bool Wait(Optional<int64> timeoutNS = {}) = 0;
        virtual void Signal() = 0;
        virtual ~Semaphore() = 0;
    };

    inline Semaphore::~Semaphore() {}
}
