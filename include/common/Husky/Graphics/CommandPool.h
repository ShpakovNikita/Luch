#pragma once

#include <Husky/Graphics/GraphicsObject.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class CommandPool : public GraphicsObject
    {
    public:
        CommandPool(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~CommandPool() = 0;

        virtual GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() = 0;
        // TODO
        // virtual RefPtr<ComputeCommandList> AllocateComputeCommandList() = 0;
    };

    inline CommandPool::~CommandPool() {}
}
