#pragma once

#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <Luch/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalCommandQueue : public CommandQueue
    {
        friend class MetalCommandPool;
    public:
        MetalCommandQueue(MetalGraphicsDevice* device, mtlpp::CommandQueue queue);
        //GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() override;
        GraphicsResultRefPtr<CommandPool> CreateCommandPool() override;

        GraphicsResult Submit(
            GraphicsCommandList* commandList) override;

        GraphicsResult Submit(
            CopyCommandList* commandList) override;

        GraphicsResult Present(
            SwapchainTexture* swapchainTexture,
            std::function<void()> presentedHandler) override;
    private:
        mtlpp::CommandQueue queue;
    };
}
