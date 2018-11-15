#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class CommandQueue : public GraphicsObject
    {
    public:
        CommandQueue(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~CommandQueue() = 0;

        virtual GraphicsResultRefPtr<CommandPool> CreateCommandPool() = 0;
        //virtual GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() = 0;

        virtual GraphicsResult Submit(
            GraphicsCommandList* commandList) = 0;

        virtual GraphicsResult Submit(
            CopyCommandList* commandList) = 0;

        virtual GraphicsResult Present(
            int32 imageIndex,
            Swapchain* swapchain) = 0;
    };

    inline CommandQueue::~CommandQueue() {}
}
