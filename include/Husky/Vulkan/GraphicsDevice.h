#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/BufferView.h>
#include <Husky/Vulkan/CommandBuffer.h>
#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/DescriptorPool.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>
#include <Husky/Vulkan/Fence.h>
#include <Husky/Vulkan/Framebuffer.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/IndexBuffer.h>
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/PipelineCache.h>
#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/RenderPass.h>
#include <Husky/Vulkan/Semaphore.h>
#include <Husky/Vulkan/ShaderModule.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/VertexBuffer.h>
#include <Husky/Vulkan/Sampler.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;
    class ShaderCompiler;

    class GraphicsDevice
    {
        friend class Buffer;
        friend class BufferView;
        friend class CommandBuffer;
        friend class CommandPool;
        friend class DescriptorPool;
        friend class DescriptorSet;
        friend class DescriptorSetLayout;
        friend class Fence;
        friend class Framebuffer;
        friend class Image;
        friend class ImageView;
        friend class PhysicalDevice;
        friend class Pipeline;
        friend class PipelineCache;
        friend class PipelineLayout;
        friend class RenderPass;
        friend class Semaphore;
        friend class ShaderModule;
        friend class Swapchain;
        friend class Sampler;
    public:
        GraphicsDevice() = default;

        GraphicsDevice(GraphicsDevice&& other);
        GraphicsDevice& operator=(GraphicsDevice&& other);

        ~GraphicsDevice();

        inline PhysicalDevice* GetPhysicalDevice() const { return physicalDevice; }
        inline vk::Device GetDevice() { return device; }
        inline const QueueIndices& GetQueueIndices() { return queueInfo.indices; }
        inline Queue* GetGraphicsQueue() { return &queueInfo.graphicsQueue; }
        inline PresentQueue* GetPresentQueue() { return &queueInfo.presentQueue; }
        inline Queue* GetComputeQueue() { return &queueInfo.computeQueue; }
        inline const vk::AllocationCallbacks& GetAllocationCallbacks() const { return allocationCallbacks; }

        vk::Result WaitIdle();

        int32 ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties);

        VulkanResultValue<Swapchain> CreateSwapchain(
            const SwapchainCreateInfo& swapchainCreateInfo,
            Surface* surface);

        VulkanResultValue<CommandPool> CreateCommandPool(QueueIndex queueIndex, bool transient = false, bool canReset = false);
        VulkanResultValue<Buffer> CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage, bool mappable);
        VulkanResultValue<BufferView> CreateBufferView(Buffer* buffer, Format format, int64 offset, int64 size);
        VulkanResultValue<IndexBuffer> CreateIndexBuffer(IndexType indexType, int32 indexCount, QueueIndex queueIndex, bool mappable);
        VulkanResultValue<VertexBuffer> CreateVertexBuffer(int32 vertexSize, int32 vertexCount, QueueIndex queueIndex, bool mappable);
        VulkanResultValue<Image> CreateImage(const vk::ImageCreateInfo& imageCreateInfo);
        VulkanResultValue<ImageView> CreateImageView(Image* image, vk::ImageViewCreateInfo& imageViewCreateInfo);
        VulkanResultValue<ImageView> CreateImageView(Image* image);
        VulkanResultValue<ShaderModule> CreateShaderModule(uint32* bytecode, int64 bytecodeSizeInBytes);
        VulkanResultValue<PipelineCache> CreatePipelineCache();

        VulkanResultValue<Pipeline> CreateGraphicsPipeline(
            const GraphicsPipelineCreateInfo& graphicsPipelineCreateInfo,
            PipelineCache* pipelineCache = nullptr);

        VulkanResultValue<RenderPass> CreateRenderPass(const RenderPassCreateInfo& createInfo);
        VulkanResultValue<DescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);
        VulkanResultValue<DescriptorPool> CreateDescriptorPool(int32 maxSets, const UnorderedMap<vk::DescriptorType, int32>& poolSizes, bool canFreeDescriptors = false);
        VulkanResultValue<PipelineLayout> CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);
        VulkanResultValue<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& createInfo);
        VulkanResultValue<Fence> CreateFence(bool signaled = false);
        VulkanResultValue<Semaphore> CreateSemaphore();
        VulkanResultValue<Sampler> CreateSampler(const vk::SamplerCreateInfo& createInfo);
    private:
        GraphicsDevice(
            PhysicalDevice* physicalDevice,
            vk::Device device,
            QueueInfo&& queueInfo,
            vk::AllocationCallbacks allocationCallbacks);

        VulkanResultValue<vk::DeviceMemory> AllocateMemory(
            vk::MemoryRequirements memoryRequirements,
            vk::MemoryPropertyFlags memoryPropertyFlags);

        void Destroy();

        void DestroySwapchain(Swapchain* swapchain);
        void DestroyCommandPool(CommandPool* commandPool);
        void DestroyBuffer(Buffer* buffer);
        void DestroyBufferView(BufferView* bufferView);
        void DestroyImage(Image* image);
        void DestroyImageView(ImageView* imageView);
        void DestroyPipeline(Pipeline* pipeline);
        void DestroyPipelineLayout(PipelineLayout* pipelineLayout);
        void DestroyPipelineCache(PipelineCache* pipelineCache);
        void DestroyShaderModule(ShaderModule* module);
        void DestroyRenderPass(RenderPass* renderPass);
        void DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
        void DestroyDescriptorPool(DescriptorPool* descriptorPool);
        void DestroyFramebuffer(Framebuffer* framebuffer);
        void DestroyFence(Fence* fence);
        void DestroySemaphore(Semaphore* semaphore);
        void DestroySampler(Sampler* sampler);

        PhysicalDevice* physicalDevice = nullptr;
        vk::Device device;
        QueueInfo queueInfo;
        vk::AllocationCallbacks allocationCallbacks;
    };

}
