#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;
    class CommandBuffer;

    class Device
    {
        friend class PhysicalDevice;
        friend class Swapchain;
        friend class CommandPool;
        friend class CommandBuffer;
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

        int32 ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties);

        VulkanResultValue<Swapchain> CreateSwapchain(
            const SwapchainCreateInfo& swapchainCreateInfo,
            Surface* surface);

        VulkanResultValue<CommandPool> CreateCommandPool(QueueIndex queueIndex, bool transient = false, bool canReset = false);

        VulkanResultValue<Buffer> CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage);
    private:
        VulkanResultValue<vk::DeviceMemory> AllocateMemory(
            int64 size,
            vk::MemoryRequirements memoryRequirements,
            vk::MemoryPropertyFlags memoryPropertyFlags);

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
