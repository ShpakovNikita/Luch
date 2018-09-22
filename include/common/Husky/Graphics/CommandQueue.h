#pragma once

#include <Husky/Graphics/GraphicsObject.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class CommandQueue : public GraphicsObject
    {
    public:
        CommandQueue(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~CommandQueue() = 0;

        virtual GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() = 0;
    };

    inline CommandQueue::~CommandQueue() {}
}
