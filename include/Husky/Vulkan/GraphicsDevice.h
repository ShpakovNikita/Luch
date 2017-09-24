#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;
    class CommandBuffer;
    class Buffer;
    class Image;
    class ImageView;

    class GraphicsDevice
    {
        friend class PhysicalDevice;
        friend class Swapchain;
        friend class CommandPool;
        friend class CommandBuffer;
        friend class Buffer;
        friend class Image;
        friend class ImageView;
    public:
        GraphicsDevice() = default;

        GraphicsDevice(GraphicsDevice&& other);
        GraphicsDevice& operator=(GraphicsDevice&& other);

        ~GraphicsDevice();

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
        VulkanResultValue<Image> CreateImage(const vk::ImageCreateInfo& imageCreateInfo);
        VulkanResultValue<ImageView> CreateImageView(Image* image, vk::ImageViewCreateInfo& imageViewCreateInfo);
        VulkanResultValue<ImageView> CreateImageView(Image* image);
    private:
        VulkanResultValue<vk::DeviceMemory> AllocateMemory(
            vk::MemoryRequirements memoryRequirements,
            vk::MemoryPropertyFlags memoryPropertyFlags);

        void DestroySwapchain(Swapchain* swapchain);
        void DestroyCommandPool(CommandPool* commandPool);
        void DestroyBuffer(Buffer* buffer);
        void DestroyImage(Image* image);
        void DestroyImageView(ImageView* imageView);

        GraphicsDevice(
            PhysicalDevice* physicalDevice,
            vk::Device device,
            QueueInfo&& queueInfo,
            vk::AllocationCallbacks allocationCallbacks);

        PhysicalDevice* physicalDevice = nullptr;
        vk::Device device;
        QueueInfo queueInfo;
        vk::AllocationCallbacks allocationCallbacks;
    };

}
