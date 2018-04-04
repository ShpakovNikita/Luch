#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Surface;
    class ShaderCompiler;

    class GraphicsDevice : public BaseObject
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
        GraphicsDevice(
            PhysicalDevice* physicalDevice,
            vk::Device device,
            QueueInfo&& queueInfo,
            vk::AllocationCallbacks allocationCallbacks);

        ~GraphicsDevice() override;

        inline vk::Device GetDevice() { return device; }
        inline const vk::AllocationCallbacks& GetAllocationCallbacks() const { return allocationCallbacks; }

        inline PhysicalDevice* GetPhysicalDevice() const { return physicalDevice; }
        inline const QueueIndices* GetQueueIndices() { return &queueInfo.indices; }
        inline Queue* GetGraphicsQueue() { return queueInfo.graphicsQueue.Get(); }
        inline PresentQueue* GetPresentQueue() { return queueInfo.presentQueue.Get(); }
        inline Queue* GetComputeQueue() { return queueInfo.computeQueue.Get(); }

        vk::Result WaitIdle();

        int32 ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties);

        VulkanRefResultValue<Swapchain> CreateSwapchain(const SwapchainCreateInfo& swapchainCreateInfo, Surface* surface);

        VulkanRefResultValue<CommandPool> CreateCommandPool(QueueIndex queueIndex, bool transient = false, bool canReset = false);
        VulkanRefResultValue<Buffer> CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage, bool mappable);
        VulkanRefResultValue<BufferView> CreateBufferView(Buffer* buffer, Format format, int64 offset, int64 size);
        VulkanRefResultValue<Image> CreateImage(const vk::ImageCreateInfo& imageCreateInfo);
        VulkanRefResultValue<ImageView> CreateImageView(Image* image, vk::ImageViewCreateInfo& imageViewCreateInfo);
        VulkanRefResultValue<ImageView> CreateImageView(Image* image);
        VulkanRefResultValue<ShaderModule> CreateShaderModule(uint32* bytecode, int64 bytecodeSizeInBytes);
        VulkanRefResultValue<PipelineCache> CreatePipelineCache();
        VulkanRefResultValue<Pipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& graphicsPipelineCreateInfo, PipelineCache* pipelineCache = nullptr);
        VulkanRefResultValue<RenderPass> CreateRenderPass(const RenderPassCreateInfo& createInfo);
        VulkanRefResultValue<DescriptorSetLayout> CreateDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);
        VulkanRefResultValue<DescriptorPool> CreateDescriptorPool(int32 maxSets, const UnorderedMap<vk::DescriptorType, int32>& poolSizes, bool canFreeDescriptors = false);
        VulkanRefResultValue<PipelineLayout> CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo);
        VulkanRefResultValue<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& createInfo);
        VulkanRefResultValue<Fence> CreateFence(bool signaled = false);
        VulkanRefResultValue<Semaphore> CreateSemaphore();
        VulkanRefResultValue<Sampler> CreateSampler(const vk::SamplerCreateInfo& createInfo);
    private:
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
