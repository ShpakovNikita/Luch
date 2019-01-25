#include <Luch/Metal/MetalCommandPool.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalCommandQueue.h>
#include <Luch/Metal/MetalGraphicsCommandList.h>
#include <Luch/Metal/MetalComputeCommandList.h>
#include <Luch/Metal/MetalCopyCommandList.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalCommandPool::MetalCommandPool(
        MetalGraphicsDevice* device,
        MetalCommandQueue* aQueue)
        : CommandPool(device)
        , queue(aQueue)
    {
    }

    GraphicsResultRefPtr<GraphicsCommandList> MetalCommandPool::AllocateGraphicsCommandList()
    {
        auto mtlGraphicsDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        auto mtlCommandBuffer = queue->queue.CommandBuffer();
        return { GraphicsResult::Success, MakeRef<MetalGraphicsCommandList>(mtlGraphicsDevice, mtlCommandBuffer) };
    }

    GraphicsResultRefPtr<ComputeCommandList> MetalCommandPool::AllocateComputeCommandList()
    {
        auto mtlGraphicsDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        auto mtlCommandBuffer = queue->queue.CommandBuffer();
        return { GraphicsResult::Success, MakeRef<MetalComputeCommandList>(mtlGraphicsDevice, mtlCommandBuffer) };
    }

    GraphicsResultRefPtr<CopyCommandList> MetalCommandPool::AllocateCopyCommandList()
    {
        auto mtlGraphicsDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        auto mtlCommandBuffer = queue->queue.CommandBuffer();
        return { GraphicsResult::Success, MakeRef<MetalCopyCommandList>(mtlGraphicsDevice, mtlCommandBuffer) };
    }
}
