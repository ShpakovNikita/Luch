#include <Husky/Metal/MetalCommandQueue.h>

namespace Husky::Metal
{
    MetalCommandQueue::MetalCommandQueue(MetalGraphicsDevice* aDevice, mtlpp::CommandQueue aQueue)
        : CommandQueue(aDevice)
        , queue(aQueue)
    {
    }

    GraphicsResultRefPtr<GraphicsCommandList> MetalCommandQueue::AllocateGraphicsCommandList()
    {
        auto metalCommandBuffer = queue.CommandBuffer();
        return { GraphicsResult::Success, MakeRef<MetalCommandList>(this, metalCommandBuffer) };
    }
}


