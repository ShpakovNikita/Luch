#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    class Semaphore : public GraphicsObject
    {
    public:
        Semaphore(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Semaphore() = 0;
    };

    inline Semaphore::~Semaphore() {}
}
