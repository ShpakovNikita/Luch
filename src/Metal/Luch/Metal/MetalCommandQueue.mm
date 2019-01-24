#include <Luch/Metal/MetalCommandQueue.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalCommandPool.h>
#include <Luch/Metal/MetalGraphicsCommandList.h>
#include <Luch/Metal/MetalCopyCommandList.h>
#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalSwapchain.h>

namespace Luch::Metal
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

    GraphicsResult MetalCommandQueue::Submit(
        CopyCommandList* commandList)
    {
        auto mtlCopyCommandList = static_cast<MetalCopyCommandList*>(commandList);

        mtlCopyCommandList->commandBuffer.Commit();
        mtlCopyCommandList->commandBuffer.WaitUntilCompleted();
        // TODO
        return GraphicsResult::Success;
    }

    GraphicsResult MetalCommandQueue::Present(
        SwapchainTexture* swapchainTexture,
        std::function<void()> presentedHandler)
    {
        auto mtlSwapchainTexture = static_cast<MetalSwapchainTexture*>(swapchainTexture);
        auto commandBuffer = queue.CommandBuffer();

        commandBuffer.AddCompletedHandler([presentedHandler](auto buffer)
        {
            presentedHandler();
        });

        commandBuffer.Present(mtlSwapchainTexture->drawable);
        commandBuffer.Commit();

        return GraphicsResult::Success;
    }
}


