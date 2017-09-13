#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/CommandPool.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;

    class Device
    {
        friend class PhysicalDevice;
        friend class Swapchain;
        friend class CommandPool;
    public:
        Device() = default;

        Device(Device&& other);
        Device& operator=(Device&& other);

        ~Device();

        inline PhysicalDevice* GetPhysicalDevice() const { return physicalDevice; }
        inline vk::Device GetDevice() { return device; }
        inline const QueueIndices& GetQueueIndices() { return queueInfo.indices; }
        inline Queue* GetGraphicsQueue() { return &queueInfo.graphicsQueue; }
        inline Queue* GetPresentQueue() { return &queueInfo.presentQueue; }
        inline Queue* GetComputeQueue() { return &queueInfo.computeQueue; }
        inline const vk::AllocationCallbacks& GetAllocationCallbacks() const { return allocationCallbacks; }

        vk::Result WaitIdle();

        VulkanResultValue<Swapchain> CreateSwapchain(
            const SwapchainCreateInfo& swapchainCreateInfo,
            Surface* surface);

        VulkanResultValue<CommandPool> CreateCommandPool(QueueIndex queueIndex, bool transient = false, bool canReset = false);
    private:
        void DestroySwapchain(Swapchain* swapchain);
        void DestroyCommandPool(CommandPool* commandPool);

        Device(
            PhysicalDevice* physicalDevice,
            vk::Device device,
            QueueInfo&& queueInfo,
            vk::AllocationCallbacks allocationCallbacks);

        vk::AllocationCallbacks allocationCallbacks;
        QueueInfo queueInfo;
        PhysicalDevice* physicalDevice = nullptr;
        vk::Device device;
    };

}
