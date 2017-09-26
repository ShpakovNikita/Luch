#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/CommandBuffer.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/PipelineCache.h>
#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/ShaderModule.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;
    class ShaderCompiler;

    class GraphicsDevice
    {
        friend class PhysicalDevice;
        friend class Swapchain;
        friend class CommandPool;
        friend class CommandBuffer;
        friend class Buffer;
        friend class Image;
        friend class ImageView;
        friend class ShaderModule;
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
        VulkanResultValue<ShaderModule> CompileShaderModule(ShaderCompiler* compiler, char8* sourceCode, int64 sourceCodeSize);
        VulkanResultValue<PipelineCache> CreatePipelineCache();
        VulkanResultValue<Pipeline> CreateGraphicsPipeline(
            const GraphicsPipelineCreateInfo& graphicsPipelineCreateInfo,
            PipelineCache* pipelineCache);
    private:
        GraphicsDevice(
            PhysicalDevice* physicalDevice,
            vk::Device device,
            QueueInfo&& queueInfo,
            vk::AllocationCallbacks allocationCallbacks);

        VulkanResultValue<vk::DeviceMemory> AllocateMemory(
            vk::MemoryRequirements memoryRequirements,
            vk::MemoryPropertyFlags memoryPropertyFlags);

        void DestroySwapchain(Swapchain* swapchain);
        void DestroyCommandPool(CommandPool* commandPool);
        void DestroyBuffer(Buffer* buffer);
        void DestroyImage(Image* image);
        void DestroyImageView(ImageView* imageView);
        void DestroyPipeline(Pipeline* pipeline);
        void DestroyPipelineCache(PipelineCache* pipelineCache);
        void DestroyShaderModule(ShaderModule* module);

        PhysicalDevice* physicalDevice = nullptr;
        vk::Device device;
        QueueInfo queueInfo;
        vk::AllocationCallbacks allocationCallbacks;
    };

}
