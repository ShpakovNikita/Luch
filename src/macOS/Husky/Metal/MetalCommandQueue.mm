#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalGraphicsCommandList.h>

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
        return { GraphicsResult::Success, MakeRef<MetalGraphicsCommandList>(this, metalCommandBuffer) };
    }
}


