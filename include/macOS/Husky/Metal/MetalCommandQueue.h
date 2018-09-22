#pragma once

#include <Husky/Graphics/CommandQueue.h>
#include <Husky/Graphics/GraphicsResultValue.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    class MetalCommandQueue : public CommandQueue
    {
    public:
        MetalCommandQueue(MetalGraphicsDevice* device, mtlpp::CommandQueue queue);
        GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() override;
    private:
        mtlpp::CommandQueue queue;
    };
}
