#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalCommandPool.h>
#include <Husky/Metal/MetalGraphicsCommandList.h>
#include <Husky/Metal/MetalSemaphore.h>
#include <Husky/Metal/MetalSwapchain.h>

namespace Husky::Metal
{
    MetalCommandQueue::MetalCommandQueue(MetalGraphicsDevice* aDevice, mtlpp::CommandQueue aQueue)
        : CommandQueue(aDevice)
        , queue(aQueue)
    {
    }

//    GraphicsResultRefPtr<GraphicsCommandList> MetalCommandQueue::AllocateGraphicsCommandList()
//    {
//        auto metalCommandBuffer = queue.CommandBuffer();
//        return { GraphicsResult::Success, MakeRef<MetalGraphicsCommandList>(this, metalCommandBuffer) };
//    }

    GraphicsResultRefPtr<CommandPool> MetalCommandQueue::CreateCommandPool()
    {
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        return { GraphicsResult::Success, MakeRef<MetalCommandPool>(mtlDevice, this) };
    }

    GraphicsResult MetalCommandQueue::Submit(
        GraphicsCommandList* commandList)
    {
        auto mtlGraphicsCommandList = static_cast<MetalGraphicsCommandList*>(commandList);

        mtlGraphicsCommandList->commandBuffer.Commit();
        mtlGraphicsCommandList->commandBuffer.WaitUntilCompleted();
        // TODO
        return GraphicsResult::Success;
    }

    GraphicsResult MetalCommandQueue::Present(
        int32 imageIndex,
        Swapchain* swapchain)
    {
        auto mtlSwapchain = static_cast<MetalSwapchain*>(swapchain);

        [mtlSwapchain->drawable present];

        return GraphicsResult::Success;
    }
}


