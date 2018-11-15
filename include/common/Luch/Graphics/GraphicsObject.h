#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Graphics/GraphicsForwards.h>

namespace Luch::Graphics
{
    class GraphicsObject : public BaseObject
    {
    public:
        GraphicsObject(GraphicsDevice* aDevice)
            : graphicsDevice(aDevice)
        {
        }

        inline GraphicsDevice* GetGraphicsDevice() { return graphicsDevice; }
    private:
        GraphicsDevice* graphicsDevice = nullptr;
    };
}
