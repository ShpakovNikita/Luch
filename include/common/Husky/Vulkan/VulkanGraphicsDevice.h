#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Format.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class VulkanGraphicsDevice : public BaseObject
    {
        friend class VulkanCommandBuffer;
        friend class VulkanCommandPool;
        friend class VulkanDescriptorPool;
        friend class VulkanDescriptorSet;
        friend class VulkanDescriptorSetLayout;
        friend class VulkanDeviceBuffer;
        friend class VulkanDeviceBufferView;
        friend class VulkanFence;
        friend class VulkanFramebuffer;
        friend class VulkanImage;
        friend class VulkanImageView;
        friend class VulkanPhysicalDevice;
        friend class VulkanPipeline;
        friend class VulkanPipelineCache;
        friend class VulkanPipelineLayout;
        friend class VulkanRenderPass;
        friend class VulkanSemaphore;
        friend class VulkanShaderModule;
        friend class VulkanSwapchain;
        friend class VulkanSampler;
    public:
        VulkanGraphicsDevice(
            VulkanPhysicalDevice* physicalDevice,
            vk::Device device,
            QueueInfo&& queueInfo,
            Husky::Optional<vk::AllocationCallbacks> allocationCallbacks);

        ~VulkanGraphicsDevice() override;

        inline vk::Device GetDevice() { return device; }
        inline const vk::Optional<const vk::AllocationCallbacks>& GetAllocationCallbacks() const { return allocationCallbacks; }

        inline VulkanPhysicalDevice* GetPhysicalDevice() const { return physicalDevice; }
        inline const QueueIndices* GetQueueIndices() { return &queueInfo.indices; }
        inline Queue* GetGraphicsQueue() { return queueInfo.graphicsQueue; }
        inline PresentQueue* GetPresentQueue() { return queueInfo.presentQueue; }
        inline Queue* GetComputeQueue() { return queueInfo.computeQueue; }

        vk::Result WaitIdle();

        int32 ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties);
        vk::ImageViewCreateInfo GetDefaultImageViewCreateInfo(Image* image);

        VulkanRefResultValue<Swapchain> CreateSwapchain(const SwapchainCreateInfo& swapchainCreateInfo, Surface* surface);

        VulkanRefResultValue<CommandPool> CreateCommandPool(QueueIndex queueIndex, bool transient = false, bool canReset = false);
        VulkanRefResultValue<DeviceBuffer> CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage, bool mappable);
        VulkanRefResultValue<DeviceBufferView> CreateBufferView(DeviceBuffer* buffer, Format format, int64 offset, int64 size);
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
        void DestroyBuffer(DeviceBuffer* buffer);
        void DestroyBufferView(DeviceBufferView* bufferView);
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

        // Workaround for vk::Optional not holding to the value itself
        Husky::Optional<vk::AllocationCallbacks> callbacks;
        vk::Optional<const vk::AllocationCallbacks> allocationCallbacks = nullptr;
    };
}
