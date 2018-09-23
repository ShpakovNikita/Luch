#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
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
