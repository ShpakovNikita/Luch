#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwads.h>

namespace Husky::Graphics
{
    class GraphicsObject : public BaseObject
    {
    public:
        GraphicsObject(GraphicsDevice* aDevice)
            : device(aDevice)
        {
        }

        inline GraphicsDevice* GetGraphicsDevice() { return device; }
    private:
        GraphicsDevice* device = nullptr;
    }
}
