#include <Husky/Vulkan/VulkanGraphicsDevice.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/Vulkan/VulkanPhysicalDevice.h>
#include <Husky/Vulkan/VulkanShaderCompiler.h>
#include <Husky/Vulkan/VulkanSurface.h>
#include <Husky/Vulkan/VulkanCommandBuffer.h>
#include <Husky/Vulkan/VulkanCommandPool.h>
#include <Husky/Vulkan/VulkanDeviceBuffer.h>
#include <Husky/Vulkan/VulkanDeviceBufferView.h>
#include <Husky/Vulkan/VulkanDescriptorPool.h>
#include <Husky/Vulkan/VulkanDescriptorSetLayout.h>
#include <Husky/Vulkan/VulkanFence.h>
#include <Husky/Vulkan/VulkanFramebuffer.h>
#include <Husky/Vulkan/VulkanImage.h>
#include <Husky/Vulkan/VulkanImageView.h>
#include <Husky/Vulkan/VulkanPipeline.h>
#include <Husky/Vulkan/PipelineCache.h>
#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/VulkanRenderPass.h>
#include <Husky/Vulkan/VulkanSemaphore.h>
#include <Husky/Vulkan/VulkanShaderModule.h>
#include <Husky/Vulkan/VulkanSwapchain.h>
#include <Husky/Vulkan/VulkanSampler.h>
#include <Husky/Vulkan/RenderPassCreateInfo.h>
#include <Husky/Vulkan/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Vulkan/PipelineLayoutCreateInfo.h>
#include <Husky/Vulkan/FramebufferCreateInfo.h>

namespace Husky::Vulkan
{
    VulkanGraphicsDevice::VulkanGraphicsDevice(
        VulkanPhysicalDevice* aPhysicalDevice,
        vk::Device aDevice,
        VulkanQueueInfo&& aQueueInfo,
        Husky::Optional<vk::AllocationCallbacks> aAllocationCallbacks)
        : physicalDevice(aPhysicalDevice)
        , device(aDevice)
        , queueInfo(std::move(aQueueInfo))
        , callbacks(aAllocationCallbacks)
        , allocationCallbacks(nullptr)
    {
        if(callbacks.has_value())
        {
            allocationCallbacks = *callbacks;
        }
    }

    VulkanGraphicsDevice::~VulkanGraphicsDevice()
    {
        Destroy();
    }

    vk::Result VulkanGraphicsDevice::WaitIdle()
    {
        return device.waitIdle();
    }

    int32 VulkanGraphicsDevice::ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties)
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

    vk::ImageViewCreateInfo VulkanGraphicsDevice::GetDefaultImageViewCreateInfo(VulkanImage* image)
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

    VulkanRefResultValue<VulkanSwapchain> VulkanGraphicsDevice::CreateSwapchain(
        const SwapchainCreateInfo& swapchainCreateInfo,
        VulkanSurface* surface)
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

            Vector<VulkanSwapchainImage> swapchainImages;
            swapchainImages.resize(swapchainImageCount);

            for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
            {
                auto vulkanImage = images[i];
                auto image = MakeRef<VulkanImage>(this, vulkanImage, nullptr, vk::ImageCreateInfo{}, vk::MemoryRequirements{}, false);

                vk::ImageSubresourceRange subresourceRange;
                subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
                subresourceRange.setLayerCount(swapchainCreateInfo.arrayLayers);
                subresourceRange.setLevelCount(1);

                vk::ImageViewCreateInfo imageViewCi;
                imageViewCi.setImage(vulkanImage);
                imageViewCi.setViewType(vk::ImageViewType::e2D);
                imageViewCi.setFormat(ToVulkanFormat(swapchainCreateInfo.format));
                imageViewCi.setSubresourceRange(subresourceRange);
                auto [imageViewCreateResult, imageView] = CreateImageView(image, imageViewCi);
                if (imageViewCreateResult != vk::Result::eSuccess)
                {
                    // TODO
                    return { imageViewCreateResult };
                }

                swapchainImages[i] = { image, imageView };
            }
            
            return
            {
                result,
                MakeRef<VulkanSwapchain>(
                    this,
                    vulkanSwapchain,
                    swapchainCreateInfo,
                    swapchainImageCount,
                    std::move(swapchainImages))
            };
        }
        else
        {
            device.destroySwapchainKHR(vulkanSwapchain);
            return { result };
        }
    }

    VulkanRefResultValue<VulkanCommandPool> VulkanGraphicsDevice::CreateCommandPool(
        QueueIndex queueIndex,
        bool transient,
        bool canReset)
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
            return { result, MakeRef<VulkanCommandPool>(this, vulkanCommandPool) };
        }
    }

    VulkanRefResultValue<VulkanDeviceBuffer> VulkanGraphicsDevice::CreateBuffer(
        int64 size,
        QueueIndex queueIndex,
        vk::BufferUsageFlags usage,
        bool mappable)
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
            return { createBufferResult, MakeRef<DeviceBuffer>(this, vulkanBuffer, vulkanMemory, ci) };
        }
    }

    VulkanRefResultValue<VulkanDeviceBufferView> VulkanGraphicsDevice::CreateBufferView(
        VulkanDeviceBuffer* buffer,
        Format format,
        int64 offset,
        int64 size)
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
            return { createResult, MakeRef<DeviceBufferView>(this, vulkanBufferView) };
        }
    }

    VulkanRefResultValue<VulkanImage> VulkanGraphicsDevice::CreateImage(const vk::ImageCreateInfo& imageCreateInfo)
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

        return
        {
            vk::Result::eSuccess,
            MakeRef<VulkanImage>(this, vulkanImage, vulkanMemory, imageCreateInfo, memoryRequirements)
        };
    }

    VulkanRefResultValue<VulkanImageView> VulkanGraphicsDevice::CreateImageView(
        VulkanImage* image,
        vk::ImageViewCreateInfo& imageViewCreateInfo)
    {
        imageViewCreateInfo.setImage(image->GetImage());
        auto[createImageViewResult, vulkanImageView] = device.createImageView(imageViewCreateInfo, allocationCallbacks);
        if (createImageViewResult != vk::Result::eSuccess)
        {
            device.destroyImageView(vulkanImageView, allocationCallbacks);
            return { createImageViewResult };
        }

        return { createImageViewResult, MakeRef<VulkanImageView>(this, vulkanImageView) };
    }

    VulkanRefResultValue<VulkanImageView> VulkanGraphicsDevice::CreateImageView(VulkanImage* image)
    {
        auto ci = GetDefaultImageViewCreateInfo(image);
        return CreateImageView(image, ci);
    }

    VulkanRefResultValue<VulkanShaderModule> VulkanGraphicsDevice::CreateShaderModule(
        uint32* bytecode,
        int64 bytecodeSizeInBytes)
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
            return { createResult, MakeRef<VulkanShaderModule>(this, vulkanShaderModule) };
        }
    }

    VulkanRefResultValue<PipelineCache> VulkanGraphicsDevice::CreatePipelineCache()
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

    VulkanRefResultValue<Pipeline> VulkanGraphicsDevice::CreateGraphicsPipeline(
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

    VulkanRefResultValue<RenderPass> VulkanGraphicsDevice::CreateRenderPass(const RenderPassCreateInfo& ci)
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

    VulkanRefResultValue<DescriptorSetLayout> VulkanGraphicsDevice::CreateDescriptorSetLayout(
        const DescriptorSetLayoutCreateInfo& ci)
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

    VulkanRefResultValue<DescriptorPool> VulkanGraphicsDevice::CreateDescriptorPool(
        int32 maxSets,
        const UnorderedMap<vk::DescriptorType, int32>& poolSizes,
        bool canFreeDescriptors)
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

    VulkanRefResultValue<PipelineLayout> VulkanGraphicsDevice::CreatePipelineLayout(
        const PipelineLayoutCreateInfo& createInfo)
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

    VulkanRefResultValue<Framebuffer> VulkanGraphicsDevice::CreateFramebuffer(
        const FramebufferCreateInfo & createInfo)
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

    VulkanRefResultValue<Fence> VulkanGraphicsDevice::CreateFence(bool signaled)
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

    VulkanRefResultValue<Semaphore> VulkanGraphicsDevice::CreateSemaphore()
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

    VulkanRefResultValue<Sampler> VulkanGraphicsDevice::CreateSampler(const vk::SamplerCreateInfo& createInfo)
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

    VulkanResultValue<vk::DeviceMemory> VulkanGraphicsDevice::AllocateMemory(
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

    void VulkanGraphicsDevice::Destroy()
    {
        if (device)
        {
            device.waitIdle();

            device.destroy(allocationCallbacks);
            device = nullptr;
        }
    }

    void VulkanGraphicsDevice::DestroySwapchain(Swapchain* swapchain)
    {
        device.destroySwapchainKHR(swapchain->swapchain, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyCommandPool(CommandPool* commandPool)
    {
        device.destroyCommandPool(commandPool->commandPool, allocationCallbacks);
    }
    void VulkanGraphicsDevice::DestroyBuffer(DeviceBuffer* buffer)
    {
        device.destroyBuffer(buffer->buffer, allocationCallbacks);
        device.freeMemory(buffer->memory, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyBufferView(DeviceBufferView* bufferView)
    {
        device.destroyBufferView(bufferView->bufferView, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyImage(Image* image)
    {
        device.destroyImage(image->image, allocationCallbacks);
        device.freeMemory(image->memory, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyImageView(ImageView* imageView)
    {
        device.destroyImageView(imageView->imageView, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyPipeline(Pipeline* pipeline)
    {
        device.destroyPipeline(pipeline->pipeline, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyPipelineLayout(PipelineLayout * pipelineLayout)
    {
        device.destroyPipelineLayout(pipelineLayout->pipelineLayout, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyPipelineCache(PipelineCache* pipelineCache)
    {
        device.destroyPipelineCache(pipelineCache->pipelineCache, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyShaderModule(ShaderModule* module)
    {
        device.destroyShaderModule(module->module, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyRenderPass(RenderPass* renderPass)
    {
        device.destroyRenderPass(renderPass->renderPass, allocationCallbacks);
    }
    void VulkanGraphicsDevice::DestroyDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout)
    {
        device.destroyDescriptorSetLayout(descriptorSetLayout->descriptorSetLayout, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyDescriptorPool(DescriptorPool* descriptorPool)
    {
        device.destroyDescriptorPool(descriptorPool->descriptorPool, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyFramebuffer(Framebuffer* framebuffer)
    {
        device.destroyFramebuffer(framebuffer->framebuffer, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyFence(Fence* fence)
    {
        device.destroyFence(fence->fence, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroySemaphore(Semaphore* semaphore)
    {
        device.destroySemaphore(semaphore->semaphore, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroySampler(Sampler* sampler)
    {
        device.destroySampler(sampler->sampler, allocationCallbacks);
    }
}
