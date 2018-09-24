#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class Semaphore : public GraphicsObject
    {
    public:
        Semaphore(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Semaphore() = 0;
    };

    inline Semaphore::~Semaphore() {}
}
