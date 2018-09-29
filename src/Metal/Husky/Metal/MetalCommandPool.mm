#include <Husky/Metal/MetalCommandPool.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalGraphicsCommandList.h>
#include <Husky/Metal/MetalCopyCommandList.h>

namespace Husky::Metal
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

    GraphicsResultRefPtr<CopyCommandList> MetalCommandPool::AllocateCopyCommandList()
    {
        auto mtlGraphicsDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        auto mtlCommandBuffer = queue->queue.CommandBuffer();
        return { GraphicsResult::Success, MakeRef<MetalCopyCommandList>(mtlGraphicsDevice, mtlCommandBuffer) };
    }
}
