#include <Luch/Metal/MetalCommandQueue.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalCommandPool.h>
#include <Luch/Metal/MetalCommandList.h>
#include <Luch/Metal/MetalCopyCommandList.h>
#include <Luch/Metal/MetalGraphicsCommandList.h>
#include <Luch/Metal/MetalComputeCommandList.h>
#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalSwapchain.h>

namespace Luch::Metal
{
    MetalCommandQueue::MetalCommandQueue(MetalGraphicsDevice* aDevice, mtlpp::CommandQueue aQueue)
        : CommandQueue(aDevice)
        , queue(aQueue)
    {
    }

    GraphicsResultRefPtr<CommandPool> MetalCommandQueue::CreateCommandPool()
    {
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        return { GraphicsResult::Success, MakeRef<MetalCommandPool>(mtlDevice, this) };
    }

    GraphicsResult MetalCommandQueue::Submit(
        CommandList* commandList,
        std::function<void()> completedHandler)
    {
        MetalCommandList* mtlCommandList = nullptr;
        switch(commandList->GetType())
        {
        case CommandListType::Graphics:
            mtlCommandList = static_cast<MetalCommandList*>(static_cast<MetalGraphicsCommandList*>(commandList));
            break;
        case CommandListType::Copy:
            mtlCommandList = static_cast<MetalCommandList*>(static_cast<MetalCopyCommandList*>(commandList));
            break;
        case CommandListType::Compute:
            mtlCommandList = static_cast<MetalCommandList*>(static_cast<MetalComputeCommandList*>(commandList));
            break;
        default:
            LUCH_ASSERT(false);
        }

        mtlCommandList->commandBuffer.AddCompletedHandler([completedHandler](auto buffer)
        {
            if(completedHandler)
            {
                completedHandler();
            }
        });
        mtlCommandList->commandBuffer.Commit();

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


