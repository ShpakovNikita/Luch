#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <functional>

namespace Luch::Graphics
{
    class CommandQueue : public GraphicsObject
    {
    public:
        CommandQueue(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~CommandQueue() = 0;

        virtual GraphicsResultRefPtr<CommandPool> CreateCommandPool() = 0;

        virtual GraphicsResult Submit(
            CommandList* commandList,
            std::function<void()> completedHandler) = 0;

        virtual GraphicsResult Present(
            SwapchainTexture* swapchainTexture,
            std::function<void()> presentedHandler) = 0;
    };

    inline CommandQueue::~CommandQueue() {}
}
