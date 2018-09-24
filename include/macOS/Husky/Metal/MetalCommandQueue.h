#pragma once

#include <Husky/Graphics/CommandQueue.h>
#include <Husky/Graphics/GraphicsResultValue.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
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

        GraphicsResult Present(
            int32 imageIndex,
            Swapchain* swapchain) override;
    private:
        mtlpp::CommandQueue queue;
    };
}
