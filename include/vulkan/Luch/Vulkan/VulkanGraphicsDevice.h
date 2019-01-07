#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanQueueInfo.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    struct GraphicsPipelineCreateInfo;
    class FramebufferCreateInfo;

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
            VulkanQueueInfo&& queueInfo,
            Luch::Optional<vk::AllocationCallbacks> allocationCallbacks);

        ~VulkanGraphicsDevice() override;

        inline vk::Device GetDevice() { return device; }
        inline const vk::Optional<const vk::AllocationCallbacks>& GetAllocationCallbacks() const { return allocationCallbacks; }

        inline VulkanPhysicalDevice* GetPhysicalDevice() const { return physicalDevice; }
        inline const QueueIndices* GetQueueIndices() { return &queueInfo.indices; }
        inline VulkanQueue* GetGraphicsQueue() { return queueInfo.graphicsQueue; }
        inline VulkanPresentQueue* GetPresentQueue() { return queueInfo.presentQueue; }
        inline VulkanQueue* GetComputeQueue() { return queueInfo.computeQueue; }

        vk::Result WaitIdle();

        int32 ChooseMemoryType(Luch::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties);
        vk::ImageViewCreateInfo GetDefaultImageViewCreateInfo(VulkanImage* image);

        VulkanRefResultValue<VulkanSwapchain> CreateSwapchain(
            const VulkanSwapchainCreateInfo& swapchainCreateInfo,
            VulkanSurface* surface);

        VulkanRefResultValue<VulkanCommandPool> CreateCommandPool(
            QueueIndex queueIndex,
            bool transient = false,
            bool canReset = false);

        VulkanRefResultValue<VulkanDeviceBuffer> CreateBuffer(
            int64 size,
            QueueIndex queueIndex,
            vk::BufferUsageFlags usage,
            bool mappable);

        VulkanRefResultValue<VulkanDeviceBufferView> CreateBufferView(
            VulkanDeviceBuffer* buffer,
            Graphics::Format format,
            int64 offset,
            int64 size);

        VulkanRefResultValue<VulkanImage> CreateImage(const vk::ImageCreateInfo& imageCreateInfo);

        VulkanRefResultValue<VulkanImageView> CreateImageView(
            VulkanImage* image,
            vk::ImageViewCreateInfo& imageViewCreateInfo);

        VulkanRefResultValue<VulkanImageView> CreateImageView(VulkanImage* image);

        VulkanRefResultValue<VulkanShaderModule> CreateShaderModule(
            uint32* bytecode,
            int64 bytecodeSizeInBytes);

        VulkanRefResultValue<VulkanPipelineCache> CreatePipelineCache();

        VulkanRefResultValue<VulkanPipeline> CreateGraphicsPipeline(
            const GraphicsPipelineCreateInfo& graphicsPipelineCreateInfo,
            VulkanPipelineCache* pipelineCache = nullptr);

        VulkanRefResultValue<VulkanRenderPass> CreateRenderPass(const VulkanRenderPassCreateInfo& createInfo);

        VulkanRefResultValue<VulkanDescriptorSetLayout> CreateDescriptorSetLayout(
            const DescriptorSetLayoutCreateInfo& createInfo);

        VulkanRefResultValue<VulkanDescriptorPool> CreateDescriptorPool(
            int32 maxSets,
            const UnorderedMap<vk::DescriptorType, int32>& poolSizes,
            bool canFreeDescriptors = false);

        VulkanRefResultValue<VulkanPipelineLayout> CreatePipelineLayout(
            const PipelineLayoutCreateInfo& createInfo);

        VulkanRefResultValue<VulkanFramebuffer> CreateFramebuffer(
            const FramebufferCreateInfo& createInfo);

        VulkanRefResultValue<VulkanFence> CreateFence(bool signaled = false);

        VulkanRefResultValue<VulkanSemaphore> CreateSemaphore();

        VulkanRefResultValue<VulkanSampler> CreateSampler(const vk::SamplerCreateInfo& createInfo);
    private:
        VulkanResultValue<vk::DeviceMemory> AllocateMemory(
            vk::MemoryRequirements memoryRequirements,
            vk::MemoryPropertyFlags memoryPropertyFlags);

        void Destroy();

        void DestroySwapchain(VulkanSwapchain* swapchain);
        void DestroyCommandPool(VulkanCommandPool* commandPool);
        void DestroyBuffer(VulkanDeviceBuffer* buffer);
        void DestroyBufferView(VulkanDeviceBufferView* bufferView);
        void DestroyImage(VulkanImage* image);
        void DestroyImageView(VulkanImageView* imageView);
        void DestroyPipeline(VulkanPipeline* pipeline);
        void DestroyPipelineLayout(VulkanPipelineLayout* pipelineLayout);
        void DestroyPipelineCache(VulkanPipelineCache* pipelineCache);
        void DestroyShaderModule(VulkanShaderModule* module);
        void DestroyRenderPass(VulkanRenderPass* renderPass);
        void DestroyDescriptorSetLayout(VulkanDescriptorSetLayout* descriptorSetLayout);
        void DestroyDescriptorPool(VulkanDescriptorPool* descriptorPool);
        void DestroyFramebuffer(VulkanFramebuffer* framebuffer);
        void DestroyFence(VulkanFence* fence);
        void DestroySemaphore(VulkanSemaphore* semaphore);
        void DestroySampler(VulkanSampler* sampler);

        VulkanPhysicalDevice* physicalDevice = nullptr;
        vk::Device device;
        VulkanQueueInfo queueInfo;

        // Workaround for vk::Optional not holding to the value itself
        Luch::Optional<vk::AllocationCallbacks> callbacks;
        vk::Optional<const vk::AllocationCallbacks> allocationCallbacks = nullptr;
    };
}
