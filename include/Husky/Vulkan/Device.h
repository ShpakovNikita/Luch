#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Swapchain.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;

    class Device
    {
        friend PhysicalDevice;
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
    private:
        Device(PhysicalDevice* aPhysicalDevice, vk::Device aDevice, QueueInfo&& queueInfo, vk::AllocationCallbacks aAllocationCallbacks);

        vk::AllocationCallbacks allocationCallbacks;
        QueueInfo queueInfo;
        PhysicalDevice* physicalDevice;
        vk::Device device;
    };

}
