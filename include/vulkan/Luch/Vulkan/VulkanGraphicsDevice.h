#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanQueueInfo.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Vulkan/VulkanPhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <Luch/Graphics/GraphicsForwards.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    struct GraphicsPipelineCreateInfo;
    class FramebufferCreateInfo;

    class VulkanGraphicsDevice : public Luch::Graphics::GraphicsDevice
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
            vk::Optional<const vk::AllocationCallbacks> allocationCallbacks);

        ~VulkanGraphicsDevice() override;

        inline PhysicalDevice* GetPhysicalDevice() override
        {
            return static_cast<PhysicalDevice*>(physicalDevice);
        }

        GraphicsResultRefPtr<Swapchain> CreateSwapchain(
            const SwapchainInfo& swapchainCreateInfo,
            Surface* surface) override;

        GraphicsResultRefPtr<Semaphore> CreateSemaphore() override;

        GraphicsResultRefPtr<Sampler> CreateSampler(const SamplerCreateInfo& createInfo) override;

        GraphicsResultRefPtr<CommandQueue> CreateCommandQueue() override;
        GraphicsResultRefPtr<PipelineState> CreatePipelineState(const PipelineStateCreateInfo&) override;
        GraphicsResultRefPtr<Texture> CreateTexture(
                    const TextureCreateInfo& createInfo) override;
        GraphicsResultRefPtr<ShaderLibrary> CreateShaderLibraryFromSource(
                    const Vector<Byte>& source,
                    const UnorderedMap<String, Variant<int32, String>>& defines) override;

        inline vk::Device GetDevice() { return device; }
        inline const vk::Optional<const vk::AllocationCallbacks>& GetAllocationCallbacks() const
        { return allocationCallbacks; }

        inline const QueueIndices* GetQueueIndices() { return &queueInfo.indices; }

        vk::Result WaitIdle();

        int32 ChooseMemoryType(Luch::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties);
        vk::ImageViewCreateInfo GetDefaultImageViewCreateInfo(VulkanImage* image);

        GraphicsResultRefPtr<Buffer> CreateBuffer(
                const BufferCreateInfo& createInfo,
                const void* initialData = nullptr) override;

        GraphicsResultRefPtr<VulkanDeviceBufferView> CreateBufferView(
            VulkanDeviceBuffer* buffer,
            Graphics::Format format,
            int64 offset,
            int64 size);

        GraphicsResultRefPtr<VulkanImage> CreateImage(const vk::ImageCreateInfo& imageCreateInfo);

        GraphicsResultRefPtr<VulkanImageView> CreateImageView(
            VulkanImage* image,
            vk::ImageViewCreateInfo& imageViewCreateInfo);

        GraphicsResultRefPtr<VulkanImageView> CreateImageView(VulkanImage* image);

        GraphicsResultRefPtr<VulkanShaderModule> CreateShaderModule(
            uint32* bytecode,
            int64 bytecodeSizeInBytes);

        GraphicsResultRefPtr<VulkanPipelineCache> CreatePipelineCache();

        GraphicsResultRefPtr<VulkanPipeline> CreateGraphicsPipeline(
            const GraphicsPipelineCreateInfo& graphicsPipelineCreateInfo,
            VulkanPipelineCache* pipelineCache = nullptr);

        GraphicsResultRefPtr<RenderPass> CreateRenderPass(const RenderPassCreateInfo& createInfo) override;

        GraphicsResultRefPtr<DescriptorSetLayout> CreateDescriptorSetLayout(
            const Graphics::DescriptorSetLayoutCreateInfo& createInfo) override;

        GraphicsResultRefPtr<DescriptorPool> CreateDescriptorPool(
            const DescriptorPoolCreateInfo&) override;

        GraphicsResultRefPtr<PipelineLayout> CreatePipelineLayout(
            const Graphics::PipelineLayoutCreateInfo& createInfo) override;

        GraphicsResultRefPtr<FrameBuffer> CreateFrameBuffer(
            const FrameBufferCreateInfo& createInfo) override;

        GraphicsResultRefPtr<VulkanFence> CreateFence(bool signaled = false);
    private:
        GraphicsResultValue<vk::DeviceMemory> AllocateMemory(
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

        vk::Optional<const vk::AllocationCallbacks> allocationCallbacks = nullptr;
    };
}
