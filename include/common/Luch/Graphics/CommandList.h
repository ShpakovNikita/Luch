#pragma once

#include <Luch/Graphics/GraphicsObject.h>

namespace Luch::Graphics
{
    enum class CommandListType
    {
        Graphics,
        Compute,
        Copy,
    };

    class CommandList : public GraphicsObject
    {
    public:
        CommandList(GraphicsDevice* device) : GraphicsObject(device) {}

        virtual CommandListType GetType() const = 0;
        virtual void SetLabel(const String& label) = 0;
    };
}
