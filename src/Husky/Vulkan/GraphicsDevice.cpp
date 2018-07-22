#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/ShaderCompiler.h>
#include <Husky/Vulkan/Surface.h>
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
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/PipelineCache.h>
#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/RenderPass.h>
#include <Husky/Vulkan/Semaphore.h>
#include <Husky/Vulkan/ShaderModule.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/Sampler.h>
#include <Husky/Vulkan/RenderPassCreateInfo.h>
#include <Husky/Vulkan/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Vulkan/PipelineLayoutCreateInfo.h>
#include <Husky/Vulkan/FramebufferCreateInfo.h>

namespace Husky::Vulkan
{
    GraphicsDevice::GraphicsDevice(
        PhysicalDevice* aPhysicalDevice,
        vk::Device aDevice,
        QueueInfo&& aQueueInfo,
        vk::AllocationCallbacks aAllocationCallbacks)
        : physicalDevice(aPhysicalDevice)
        , device(aDevice)
        , queueInfo(std::move(aQueueInfo))
        , allocationCallbacks(aAllocationCallbacks)
    {
    }

    GraphicsDevice::~GraphicsDevice()
    {
        Destroy();
    }

    vk::Result GraphicsDevice::WaitIdle()
    {
        return device.waitIdle();
    }

    int32 GraphicsDevice::ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties)
    {
        auto& physicalDeviceMemoryProperties = physicalDevice->GetPhysicalDeviceMemoryProperties();

        for (int32 i = 0; i < (int32)physicalDeviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((memoryTypeBits & (1 << i)) && ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties))
            {
                return i;
            }
        }

        return -1;
    }

    vk::ImageViewCreateInfo GraphicsDevice::GetDefaultImageViewCreateInfo(Image * image)
    {
        const vk::ImageCreateInfo& imageCi = image->createInfo;

        vk::ImageSubresourceRange subresourceRange;
        vk::ImageViewCreateInfo ci;

        ci.setFormat(imageCi.format);
        ci.setImage(image->image);

        switch (imageCi.imageType)
        {
        case vk::ImageType::e1D:
            if (imageCi.arrayLayers > 1)
            {
                ci.setViewType(vk::ImageViewType::e1DArray);
            }
            else
            {
                ci.setViewType(vk::ImageViewType::e1D);
            }
            break;
        case vk::ImageType::e2D:
            if (imageCi.arrayLayers > 1)
            {
                if ((imageCi.flags & vk::ImageCreateFlagBits::eCubeCompatible) == vk::ImageCreateFlagBits::eCubeCompatible)
                {
                    auto cubeCount = imageCi.arrayLayers / 6;
                    if (cubeCount >= 2)
                    {
                        ci.setViewType(vk::ImageViewType::eCubeArray);
                        subresourceRange.setLayerCount(6 * cubeCount);
                    }
                    else
                    {
                        ci.setViewType(vk::ImageViewType::eCube);
                        subresourceRange.setLayerCount(6);
                    }
                }
                else
                {
                    ci.setViewType(vk::ImageViewType::e2DArray);
                }
            }
            else
            {
                ci.setViewType(vk::ImageViewType::e2D);
            }
            break;
        case vk::ImageType::e3D:
            ci.setViewType(vk::ImageViewType::e3D);
            break;
        }

        auto imageAspects = image->GetImageAspects();

        vk::ImageAspectFlags vulkanAspects;

        if ((imageAspects & ImageAspects::Color) == ImageAspects::Color)
        {
            vulkanAspects |= vk::ImageAspectFlagBits::eColor;
        }
        else
        {
            if ((imageAspects & ImageAspects::Depth) == ImageAspects::Depth)
            {
                vulkanAspects |= vk::ImageAspectFlagBits::eDepth;
            }

            if ((imageAspects & ImageAspects::Stencil) == ImageAspects::Stencil)
            {
                vulkanAspects |= vk::ImageAspectFlagBits::eStencil;
            }
        }

        subresourceRange.setAspectMask(vulkanAspects);
        subresourceRange.setBaseArrayLayer(0);
        subresourceRange.setBaseMipLevel(0);
        subresourceRange.setLevelCount(imageCi.mipLevels);
        subresourceRange.setLayerCount(imageCi.arrayLayers);

        ci.setSubresourceRange(subresourceRange);
        return ci;
    }

    VulkanRefResultValue<Swapchain> GraphicsDevice::CreateSwapchain(const SwapchainCreateInfo& swapchainCreateInfo, Surface* surface)
    {
        vk::SwapchainCreateInfoKHR ci;
        ci.setImageColorSpace(swapchainCreateInfo.colorSpace);
        ci.setImageFormat(ToVulkanFormat(swapchainCreateInfo.format));
        ci.setMinImageCount(swapchainCreateInfo.imageCount);
        ci.setPresentMode(swapchainCreateInfo.presentMode);
        ci.setImageArrayLayers(swapchainCreateInfo.arrayLayers);
        ci.setImageExtent({ static_cast<uint32>(swapchainCreateInfo.width), static_cast<uint32>(swapchainCreateInfo.height) });
        ci.setSurface(surface->GetSurface());
        ci.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
        // TODO figure out composite alpha
        if (queueInfo.indices.presentQueueFamilyIndex == queueInfo.indices.graphicsQueueFamilyIndex)
        {
            uint32 indices[] = { queueInfo.indices.presentQueueFamilyIndex };
            ci.setQueueFamilyIndexCount(1);
            ci.setPQueueFamilyIndices(indices);
            ci.setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else
        {
            uint32 indices[] = { queueInfo.indices.graphicsQueueFamilyIndex, queueInfo.indices.presentQueueFamilyIndex };
            ci.setQueueFamilyIndexCount(2);
            ci.setPQueueFamilyIndices(indices);
            ci.setImageSharingMode(vk::SharingMode::eConcurrent);
        }

        auto [result, vulkanSwapchain] = device.createSwapchainKHR(ci, allocationCallbacks);
        if (result == vk::Result::eSuccess)
        {
            auto [getImagesResult, images] = device.getSwapchainImagesKHR(vulkanSwapchain);
            if (getImagesResult != vk::Result::eSuccess)
            {
                device.destroySwapchainKHR(vulkanSwapchain);
                return { result };
            }

            auto swapchainImageCount = (int32)images.size();

            Vector<SwapchainImage> swapchainImages;
            swapchainImages.resize(swapchainImageCount);

            for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
            {
                auto vulkanImage = images[i];
                auto image = MakeRef<Image>(this, vulkanImage, nullptr, vk::ImageCreateInfo{}, vk::MemoryRequirements{}, false);

                vk::ImageSubresourceRange subresourceRange;
                subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
                subresourceRange.setLayerCount(swapchainCreateInfo.arrayLayers);
                subresourceRange.setLevelCount(1);

                vk::ImageViewCreateInfo imageViewCi;
                imageViewCi.setImage(vulkanImage);
                imageViewCi.setViewType(vk::ImageViewType::e2D);
                imageViewCi.setFormat(ToVulkanFormat(swapchainCreateInfo.format));
                imageViewCi.setSubresourceRange(subresourceRange);
                auto [imageViewCreateResult, imageView] = CreateImageView(image.Get(), imageViewCi);
                if (imageViewCreateResult != vk::Result::eSuccess)
                {
                    // TODO
                    return { imageViewCreateResult };
                }

                swapchainImages[i] = { image, imageView };
            }
            
            return { result, MakeRef<Swapchain>(this, vulkanSwapchain, swapchainCreateInfo, swapchainImageCount, std::move(swapchainImages)) };
        }
        else
        {
            device.destroySwapchainKHR(vulkanSwapchain);
            return { result };
        }
    }

    VulkanRefResultValue<CommandPool> GraphicsDevice::CreateCommandPool(QueueIndex queueIndex, bool transient, bool canReset)
    {
        vk::CommandPoolCreateInfo ci;

        if (transient)
        {
            ci.flags |= vk::CommandPoolCreateFlagBits::eTransient;
        }

        if (canReset)
        {
            ci.flags |= vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        }

        ci.setQueueFamilyIndex(queueIndex);
        
        auto [result, vulkanCommandPool] = device.createCommandPool(ci, allocationCallbacks);

        if (result != vk::Result::eSuccess)
        {
            device.destroyCommandPool(vulkanCommandPool, allocationCallbacks);
            return { result };
        }
        else
        {
            return { result, MakeRef<CommandPool>(this, vulkanCommandPool) };
        }
    }

    VulkanRefResultValue<Buffer> GraphicsDevice::CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage, bool mappable)
    {
        vk::BufferCreateInfo ci;
        ci.setPQueueFamilyIndices(&queueIndex);
        ci.setQueueFamilyIndexCount(1);
        ci.setSharingMode(vk::SharingMode::eExclusive);
        ci.setSize(size);
        ci.setUsage(usage);

        auto [createBufferResult, vulkanBuffer] = device.createBuffer(ci, allocationCallbacks);
        if (createBufferResult != vk::Result::eSuccess)
        {
            device.destroyBuffer(vulkanBuffer, allocationCallbacks);
            return { createBufferResult };
        }
        
        auto memoryRequirements = device.getBufferMemoryRequirements(vulkanBuffer);

        vk::MemoryPropertyFlags memoryProperties = mappable ? vk::MemoryPropertyFlagBits::eHostVisible : vk::MemoryPropertyFlagBits::eDeviceLocal;

        auto [allocateMemoryResult, vulkanMemory] = AllocateMemory(memoryRequirements, memoryProperties);
        if (allocateMemoryResult != vk::Result::eSuccess)
        {
            device.destroyBuffer(vulkanBuffer, allocationCallbacks);
            return { allocateMemoryResult };
        }

        auto bindResult = device.bindBufferMemory(vulkanBuffer, vulkanMemory, 0);
        if (bindResult != vk::Result::eSuccess)
        {
            device.destroyBuffer(vulkanBuffer, allocationCallbacks);
            return { bindResult };
        }
        else
        {
            return { createBufferResult, MakeRef<Buffer>(this, vulkanBuffer, vulkanMemory, ci) };
        }
    }

    VulkanRefResultValue<BufferView> GraphicsDevice::CreateBufferView(Buffer* buffer, Format format, int64 offset, int64 size)
    {
        vk::BufferViewCreateInfo createInfo;
        createInfo.setBuffer(buffer->buffer);
        createInfo.setFormat(ToVulkanFormat(format));
        createInfo.setOffset(offset);
        createInfo.setRange(size);
        
        auto [createResult, vulkanBufferView] = device.createBufferView(createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyBufferView(vulkanBufferView, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<BufferView>(this, vulkanBufferView) };
        }
    }

    VulkanRefResultValue<Image> GraphicsDevice::CreateImage(const vk::ImageCreateInfo& imageCreateInfo)
    {
        auto [createImageResult, vulkanImage] = device.createImage(imageCreateInfo, allocationCallbacks);
        if(createImageResult != vk::Result::eSuccess)
        {
            device.destroyImage(vulkanImage, allocationCallbacks);
            return { createImageResult };
        }

        auto memoryRequirements = device.getImageMemoryRequirements(vulkanImage);

        auto [allocateMemoryResult, vulkanMemory] = AllocateMemory(memoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);
        if (allocateMemoryResult != vk::Result::eSuccess)
        {
            device.destroyImage(vulkanImage, allocationCallbacks);
            return { allocateMemoryResult };
        }

        auto bindResult = device.bindImageMemory(vulkanImage, vulkanMemory, 0);
        if (bindResult != vk::Result::eSuccess)
        {
            device.destroyImage(vulkanImage, allocationCallbacks);
            return { bindResult };
        }

        return { vk::Result::eSuccess, MakeRef<Image>(this, vulkanImage, vulkanMemory, imageCreateInfo, memoryRequirements) };
    }

    VulkanRefResultValue<ImageView> GraphicsDevice::CreateImageView(Image* image, vk::ImageViewCreateInfo& imageViewCreateInfo)
    {
        imageViewCreateInfo.setImage(image->GetImage());
        auto[createImageViewResult, vulkanImageView] = device.createImageView(imageViewCreateInfo, allocationCallbacks);
        if (createImageViewResult != vk::Result::eSuccess)
        {
            device.destroyImageView(vulkanImageView, allocationCallbacks);
            return { createImageViewResult };
        }
        
        return { createImageViewResult, MakeRef<ImageView>(this, vulkanImageView) };
    }

    VulkanRefResultValue<ImageView> GraphicsDevice::CreateImageView(Image* image)
    {
        auto ci = GetDefaultImageViewCreateInfo(image);
        return CreateImageView(image, ci);
    }

    VulkanRefResultValue<ShaderModule> GraphicsDevice::CreateShaderModule(uint32* bytecode, int64 bytecodeSizeInBytes)
    {
        vk::ShaderModuleCreateInfo ci;
        ci.setPCode(bytecode);
        ci.setCodeSize(bytecodeSizeInBytes);
        auto [createResult, vulkanShaderModule] = device.createShaderModule(ci, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyShaderModule(vulkanShaderModule, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<ShaderModule>(this, vulkanShaderModule) };
        }
    }

    VulkanRefResultValue<PipelineCache> GraphicsDevice::CreatePipelineCache()
    {
        vk::PipelineCacheCreateInfo ci;
        auto [createResult, vulkanPipelineCache] = device.createPipelineCache(ci, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyPipelineCache(vulkanPipelineCache, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<PipelineCache>(this, vulkanPipelineCache) };
        }
    }

    VulkanRefResultValue<Pipeline> GraphicsDevice::CreateGraphicsPipeline(
        const GraphicsPipelineCreateInfo & graphicsPipelineCreateInfo,
        PipelineCache* pipelineCache)
    {
        auto vkci = GraphicsPipelineCreateInfo::ToVulkanCreateInfo(graphicsPipelineCreateInfo);

        vk::PipelineCache cache = pipelineCache ? pipelineCache->pipelineCache : nullptr;

        auto[createResult, vulkanPipeline] = device.createGraphicsPipeline(cache, vkci->createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyPipeline(vulkanPipeline, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<Pipeline>(this, vulkanPipeline) };
        }
    }

    VulkanRefResultValue<RenderPass> GraphicsDevice::CreateRenderPass(const RenderPassCreateInfo & ci)
    {
        auto vkci = RenderPassCreateInfo::ToVulkanCreateInfo(ci);
        auto [createResult, vulkanRenderPass] = device.createRenderPass(vkci.createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyRenderPass(vulkanRenderPass, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<RenderPass>(this, vulkanRenderPass, (int32)vkci.attachments.size()) };
        }
    }

    VulkanRefResultValue<DescriptorSetLayout> GraphicsDevice::CreateDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& ci)
    {
        auto vkci = DescriptorSetLayoutCreateInfo::ToVulkanCreateInfo(ci);
        auto [createResult, vulkanDescriptorSetLayout] = device.createDescriptorSetLayout(vkci.createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyDescriptorSetLayout(vulkanDescriptorSetLayout, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<DescriptorSetLayout>(this, vulkanDescriptorSetLayout) };
        }
    }

    VulkanRefResultValue<DescriptorPool> GraphicsDevice::CreateDescriptorPool(int32 maxSets, const UnorderedMap<vk::DescriptorType, int32>& poolSizes, bool canFreeDescriptors)
    {
        auto count = (int32)poolSizes.size();
        Vector<vk::DescriptorPoolSize> vulkanPoolSizes;
        vulkanPoolSizes.reserve(count);

        for (auto& poolSize : poolSizes)
        {
            vulkanPoolSizes.emplace_back(poolSize.first, poolSize.second);
        }


        vk::DescriptorPoolCreateFlags flags;
        if (canFreeDescriptors)
        {
            flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        }

        vk::DescriptorPoolCreateInfo createInfo;
        createInfo.setMaxSets(maxSets);
        createInfo.setPoolSizeCount(count);
        createInfo.setPPoolSizes(vulkanPoolSizes.data());
        createInfo.setFlags(flags);

        auto[createResult, vulkanDescriptorPool] = device.createDescriptorPool(createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyDescriptorPool(vulkanDescriptorPool, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<DescriptorPool>(this, vulkanDescriptorPool) };
        }
    }

    VulkanRefResultValue<PipelineLayout> GraphicsDevice::CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo)
    {
        auto vkci = PipelineLayoutCreateInfo::ToVulkanCreateInfo(createInfo);
        auto [createResult, vulkanPipelineLayout] = device.createPipelineLayout(vkci.createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyPipelineLayout(vulkanPipelineLayout, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<PipelineLayout>(this, vulkanPipelineLayout) };
        }
    }

    VulkanRefResultValue<Framebuffer> GraphicsDevice::CreateFramebuffer(const FramebufferCreateInfo & createInfo)
    {
        auto vkci = FramebufferCreateInfo::ToVulkanCreateInfo(createInfo);
        auto [createResult, vulkanFramebuffer] = device.createFramebuffer(vkci, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyFramebuffer(vulkanFramebuffer, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<Framebuffer>(this, vulkanFramebuffer) };
        }
    }

    VulkanRefResultValue<Fence> GraphicsDevice::CreateFence(bool signaled)
    {
        vk::FenceCreateInfo vkci;
        if (signaled)
        {
            vkci.setFlags(vk::FenceCreateFlagBits::eSignaled);
        }

        auto [createResult, vulkanFence] = device.createFence(vkci, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyFence(vulkanFence, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<Fence>(this, vulkanFence) };
        }
    }

    VulkanRefResultValue<Semaphore> GraphicsDevice::CreateSemaphore()
    {
        vk::SemaphoreCreateInfo vkci;
        
        auto [createResult, vulkanSemaphore] = device.createSemaphore(vkci, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroySemaphore(vulkanSemaphore, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<Semaphore>(this, vulkanSemaphore) };
        }
    }

    VulkanRefResultValue<Sampler> GraphicsDevice::CreateSampler(const vk::SamplerCreateInfo& createInfo)
    {
        auto [createResult, vulkanSampler] = device.createSampler(createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroySampler(vulkanSampler, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<Sampler>(this, vulkanSampler) };
        }
    }

    VulkanResultValue<vk::DeviceMemory> GraphicsDevice::AllocateMemory(
        vk::MemoryRequirements memoryRequirements,
        vk::MemoryPropertyFlags memoryPropertyFlags)
    {
        auto memoryTypeIndex = ChooseMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);
        if (memoryTypeIndex < 0)
        {
            // TODO figure out better error for this or just make an assert
            return { vk::Result::eErrorOutOfDeviceMemory };
        }

        vk::MemoryAllocateInfo allocateInfo;
        allocateInfo.setAllocationSize(memoryRequirements.size);
        allocateInfo.setMemoryTypeIndex(memoryTypeIndex);

        auto[result, vulkanMemory] = device.allocateMemory(allocateInfo, allocationCallbacks);
        if (result == vk::Result::eSuccess)
        {
            return { result, vulkanMemory };
        }
        else
        {
            device.freeMemory(vulkanMemory, allocationCallbacks);
            return { result };
        }
    }

    void GraphicsDevice::Destroy()
    {
        if (device)
        {
            device.waitIdle();

            device.destroy(allocationCallbacks);
            device = nullptr;
        }
    }

    void GraphicsDevice::DestroySwapchain(Swapchain* swapchain)
    {
        device.destroySwapchainKHR(swapchain->swapchain, allocationCallbacks);
    }

    void GraphicsDevice::DestroyCommandPool(CommandPool* commandPool)
    {
        device.destroyCommandPool(commandPool->commandPool, allocationCallbacks);
    }
    void GraphicsDevice::DestroyBuffer(Buffer* buffer)
    {
        device.freeMemory(buffer->memory, allocationCallbacks);
        device.destroyBuffer(buffer->buffer, allocationCallbacks);
    }

    void GraphicsDevice::DestroyBufferView(BufferView* bufferView)
    {
        device.destroyBufferView(bufferView->bufferView, allocationCallbacks);
    }

    void GraphicsDevice::DestroyImage(Image* image)
    {
        device.freeMemory(image->memory, allocationCallbacks);
        device.destroyImage(image->image, allocationCallbacks);
    }

    void GraphicsDevice::DestroyImageView(ImageView* imageView)
    {
        device.destroyImageView(imageView->imageView, allocationCallbacks);
    }

    void GraphicsDevice::DestroyPipeline(Pipeline* pipeline)
    {
        device.destroyPipeline(pipeline->pipeline, allocationCallbacks);
    }

    void GraphicsDevice::DestroyPipelineLayout(PipelineLayout * pipelineLayout)
    {
        device.destroyPipelineLayout(pipelineLayout->pipelineLayout, allocationCallbacks);
    }

    void GraphicsDevice::DestroyPipelineCache(PipelineCache* pipelineCache)
    {
        device.destroyPipelineCache(pipelineCache->pipelineCache, allocationCallbacks);
    }

    void GraphicsDevice::DestroyShaderModule(ShaderModule* module)
    {
        device.destroyShaderModule(module->module, allocationCallbacks);
    }

    void GraphicsDevice::DestroyRenderPass(RenderPass* renderPass)
    {
        device.destroyRenderPass(renderPass->renderPass, allocationCallbacks);
    }
    void GraphicsDevice::DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout)
    {
        device.destroyDescriptorSetLayout(descriptorSetLayout->descriptorSetLayout, allocationCallbacks);
    }

    void GraphicsDevice::DestroyDescriptorPool(DescriptorPool* descriptorPool)
    {
        device.destroyDescriptorPool(descriptorPool->descriptorPool, allocationCallbacks);
    }

    void GraphicsDevice::DestroyFramebuffer(Framebuffer* framebuffer)
    {
        device.destroyFramebuffer(framebuffer->framebuffer, allocationCallbacks);
    }

    void GraphicsDevice::DestroyFence(Fence* fence)
    {
        device.destroyFence(fence->fence, allocationCallbacks);
    }

    void GraphicsDevice::DestroySemaphore(Semaphore* semaphore)
    {
        device.destroySemaphore(semaphore->semaphore, allocationCallbacks);
    }

    void GraphicsDevice::DestroySampler(Sampler* sampler)
    {
        device.destroySampler(sampler->sampler, allocationCallbacks);
    }
}
