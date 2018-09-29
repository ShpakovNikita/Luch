#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalCommandPool.h>
#include <Husky/Metal/MetalGraphicsCommandList.h>
#include <Husky/Metal/MetalCopyCommandList.h>
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
        int32 imageIndex,
        Swapchain* swapchain)
    {
        auto mtlSwapchain = static_cast<MetalSwapchain*>(swapchain);

        auto commandBuffer = queue.CommandBuffer();
        commandBuffer.Present(ns::Handle{ (__bridge void*)mtlSwapchain->drawable });
        commandBuffer.Commit();
        commandBuffer.WaitUntilCompleted();

        return GraphicsResult::Success;
    }
}

