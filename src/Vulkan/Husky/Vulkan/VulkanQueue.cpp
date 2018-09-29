#include <Husky/Vulkan/VulkanQueue.h>
#include <Husky/Vulkan/VulkanCommandBuffer.h>
#include <Husky/Vulkan/VulkanFence.h>
#include <Husky/Vulkan/VulkanSemaphore.h>
#include <Husky/Vulkan/VulkanSwapchain.h>

namespace Husky::Vulkan
{
    VulkanQueue::VulkanQueue(vk::Queue aQueue)
        : queue(aQueue)
    {
    }

    vk::Result VulkanQueue::WaitIdle()
    {
        return queue.waitIdle();
    }

    vk::Result VulkanQueue::Submit(const VulkanSubmission& submission)
    {
        vk::Fence vulkanFence = submission.fence ? submission.fence->GetFence() : nullptr;

        vk::SubmitInfo submitInfo;

        Vector<vk::CommandBuffer> vulkanCommandBuffers;
        vulkanCommandBuffers.reserve(submission.commandBuffers.size());

        for (auto& commandBuffer : submission.commandBuffers)
        {
            vulkanCommandBuffers.push_back(commandBuffer->GetCommandBuffer());
        }

        Vector<vk::Semaphore> vulkanWaitSemaphores;
        Vector<vk::PipelineStageFlags> vulkanWaitStages;
        vulkanWaitSemaphores.reserve(submission.waitOperations.size());
        vulkanWaitStages.reserve(submission.waitOperations.size());
        for (auto& waitOperation : submission.waitOperations)
        {
            vulkanWaitSemaphores.push_back(waitOperation.semaphore->GetSemaphore());
            vulkanWaitStages.push_back(waitOperation.stage);
        }

        Vector<vk::Semaphore> vulkanSignalSemaphores;
        vulkanSignalSemaphores.reserve(submission.signalSemaphores.size());
        for (auto& signalSemaphore : submission.signalSemaphores)
        {
            vulkanSignalSemaphores.push_back(signalSemaphore->GetSemaphore());
        }

        submitInfo.setWaitSemaphoreCount((int32)vulkanWaitSemaphores.size());
        submitInfo.setPWaitSemaphores(vulkanWaitSemaphores.data());
        submitInfo.setPWaitDstStageMask(vulkanWaitStages.data());
        submitInfo.setSignalSemaphoreCount((int32)vulkanSignalSemaphores.size());
        submitInfo.setPSignalSemaphores(vulkanSignalSemaphores.data());
        submitInfo.setCommandBufferCount((int32)vulkanCommandBuffers.size());
        submitInfo.setPCommandBuffers(vulkanCommandBuffers.data());

        return queue.submit({ submitInfo }, vulkanFence);
    }

    vk::Result VulkanPresentQueue::Present(const VulkanPresentSubmission& presentSubmission)
    {
        vk::PresentInfoKHR presentInfo;

        Vector<vk::Semaphore> vulkanWaitSemaphores;
        vulkanWaitSemaphores.reserve(presentSubmission.waitSemaphores.size());
        for (auto& waitSemaphore : presentSubmission.waitSemaphores)
        {
            vulkanWaitSemaphores.push_back(waitSemaphore->GetSemaphore());
        }

        vk::SwapchainKHR swapchain = presentSubmission.swapchain->GetSwapchain();
        uint32 index = presentSubmission.index;

        presentInfo.setSwapchainCount(1);
        presentInfo.setPSwapchains(&swapchain);
        presentInfo.setPImageIndices(&index);
        presentInfo.setWaitSemaphoreCount((int32)vulkanWaitSemaphores.size());
        presentInfo.setPWaitSemaphores(vulkanWaitSemaphores.data());

        return queue.presentKHR(presentInfo);
    }
}
