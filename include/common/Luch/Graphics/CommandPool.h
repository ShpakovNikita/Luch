#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class CommandPool : public GraphicsObject
    {
    public:
        CommandPool(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~CommandPool() = 0;

        virtual GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() = 0;
        virtual GraphicsResultRefPtr<CopyCommandList> AllocateCopyCommandList() = 0;
        // TODO
        // virtual RefPtr<ComputeCommandList> AllocateComputeCommandList() = 0;
    };

    inline CommandPool::~CommandPool() {}
}
