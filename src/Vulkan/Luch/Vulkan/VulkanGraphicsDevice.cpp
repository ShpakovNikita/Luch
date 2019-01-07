#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanFormat.h>
#include <Luch/Vulkan/VulkanPhysicalDevice.h>
#include <Luch/Vulkan/VulkanShaderCompiler.h>
#include <Luch/Vulkan/VulkanSurface.h>
#include <Luch/Vulkan/VulkanCommandBuffer.h>
#include <Luch/Vulkan/VulkanCommandPool.h>
#include <Luch/Vulkan/VulkanDeviceBuffer.h>
#include <Luch/Vulkan/VulkanDeviceBufferView.h>
#include <Luch/Vulkan/VulkanDescriptorPool.h>
#include <Luch/Vulkan/VulkanDescriptorSetLayout.h>
#include <Luch/Vulkan/VulkanFence.h>
#include <Luch/Vulkan/VulkanFramebuffer.h>
#include <Luch/Vulkan/VulkanImage.h>
#include <Luch/Vulkan/VulkanImageView.h>
#include <Luch/Vulkan/VulkanPipeline.h>
#include <Luch/Vulkan/VulkanPipelineCache.h>
#include <Luch/Vulkan/PipelineCreateInfo.h>
#include <Luch/Vulkan/VulkanPipelineLayout.h>
#include <Luch/Vulkan/VulkanRenderPass.h>
#include <Luch/Vulkan/VulkanSemaphore.h>
#include <Luch/Vulkan/VulkanShaderModule.h>
#include <Luch/Vulkan/VulkanSwapchain.h>
#include <Luch/Vulkan/VulkanSampler.h>
#include <Luch/Vulkan/VulkanRenderPassCreateInfo.h>
#include <Luch/Vulkan/VulkanDescriptorSetLayoutCreateInfo.h>
#include <Luch/Vulkan/VulkanPipelineLayoutCreateInfo.h>
#include <Luch/Vulkan/FramebufferCreateInfo.h>

namespace Luch::Vulkan
{
    VulkanGraphicsDevice::VulkanGraphicsDevice(
        VulkanPhysicalDevice* aPhysicalDevice,
        vk::Device aDevice,
        VulkanQueueInfo&& aQueueInfo,
        Luch::Optional<vk::AllocationCallbacks> aAllocationCallbacks)
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

    int32 VulkanGraphicsDevice::ChooseMemoryType(Luch::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties)
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
        const VulkanSwapchainCreateInfo& swapchainCreateInfo,
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
            return { createBufferResult, MakeRef<VulkanDeviceBuffer>(this, vulkanBuffer, vulkanMemory, ci) };
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
            return { createResult, MakeRef<VulkanDeviceBufferView>(this, vulkanBufferView) };
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

    VulkanRefResultValue<VulkanPipelineCache> VulkanGraphicsDevice::CreatePipelineCache()
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
            return { createResult, MakeRef<VulkanPipelineCache>(this, vulkanPipelineCache) };
        }
    }

    VulkanRefResultValue<VulkanPipeline> VulkanGraphicsDevice::CreateGraphicsPipeline(
        const GraphicsPipelineCreateInfo & graphicsPipelineCreateInfo,
        VulkanPipelineCache* pipelineCache)
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
            return { createResult, MakeRef<VulkanPipeline>(this, vulkanPipeline) };
        }
    }

    VulkanRefResultValue<VulkanRenderPass> VulkanGraphicsDevice::CreateRenderPass(const VulkanRenderPassCreateInfo& ci)
    {
        auto vkci = VulkanRenderPassCreateInfo::ToVulkanCreateInfo(ci);
        auto [createResult, vulkanRenderPass] = device.createRenderPass(vkci.createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyRenderPass(vulkanRenderPass, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<VulkanRenderPass>(this, vulkanRenderPass, (int32)vkci.attachments.size()) };
        }
    }

    VulkanRefResultValue<VulkanDescriptorSetLayout> VulkanGraphicsDevice::CreateDescriptorSetLayout(
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
            return { createResult, MakeRef<VulkanDescriptorSetLayout>(this, vulkanDescriptorSetLayout) };
        }
    }

    VulkanRefResultValue<VulkanDescriptorPool> VulkanGraphicsDevice::CreateDescriptorPool(
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
            return { createResult, MakeRef<VulkanDescriptorPool>(this, vulkanDescriptorPool) };
        }
    }

    VulkanRefResultValue<VulkanPipelineLayout> VulkanGraphicsDevice::CreatePipelineLayout(
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
            return { createResult, MakeRef<VulkanPipelineLayout>(this, vulkanPipelineLayout) };
        }
    }

    VulkanRefResultValue<VulkanFramebuffer> VulkanGraphicsDevice::CreateFramebuffer(
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
            return { createResult, MakeRef<VulkanFramebuffer>(this, vulkanFramebuffer) };
        }
    }

    VulkanRefResultValue<VulkanFence> VulkanGraphicsDevice::CreateFence(bool signaled)
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
            return { createResult, MakeRef<VulkanFence>(this, vulkanFence) };
        }
    }

    VulkanRefResultValue<VulkanSemaphore> VulkanGraphicsDevice::CreateSemaphore()
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
            return { createResult, MakeRef<VulkanSemaphore>(this, vulkanSemaphore) };
        }
    }

    VulkanRefResultValue<VulkanSampler> VulkanGraphicsDevice::CreateSampler(const vk::SamplerCreateInfo& createInfo)
    {
        auto [createResult, vulkanSampler] = device.createSampler(createInfo, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroySampler(vulkanSampler, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, MakeRef<VulkanSampler>(this, vulkanSampler) };
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

    void VulkanGraphicsDevice::DestroySwapchain(VulkanSwapchain* swapchain)
    {
        device.destroySwapchainKHR(swapchain->swapchain, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyCommandPool(VulkanCommandPool* commandPool)
    {
        device.destroyCommandPool(commandPool->commandPool, allocationCallbacks);
    }
    void VulkanGraphicsDevice::DestroyBuffer(VulkanDeviceBuffer* buffer)
    {
        device.destroyBuffer(buffer->buffer, allocationCallbacks);
        device.freeMemory(buffer->memory, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyBufferView(VulkanDeviceBufferView* bufferView)
    {
        device.destroyBufferView(bufferView->bufferView, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyImage(VulkanImage* image)
    {
        device.destroyImage(image->image, allocationCallbacks);
        device.freeMemory(image->memory, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyImageView(VulkanImageView* imageView)
    {
        device.destroyImageView(imageView->imageView, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyPipeline(VulkanPipeline* pipeline)
    {
        device.destroyPipeline(pipeline->pipeline, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyPipelineLayout(VulkanPipelineLayout* pipelineLayout)
    {
        device.destroyPipelineLayout(pipelineLayout->pipelineLayout, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyPipelineCache(VulkanPipelineCache* pipelineCache)
    {
        device.destroyPipelineCache(pipelineCache->pipelineCache, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyShaderModule(VulkanShaderModule* module)
    {
        device.destroyShaderModule(module->module, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyRenderPass(VulkanRenderPass* renderPass)
    {
        device.destroyRenderPass(renderPass->renderPass, allocationCallbacks);
    }
    void VulkanGraphicsDevice::DestroyDescriptorSetLayout(VulkanDescriptorSetLayout* descriptorSetLayout)
    {
        device.destroyDescriptorSetLayout(descriptorSetLayout->descriptorSetLayout, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyDescriptorPool(VulkanDescriptorPool* descriptorPool)
    {
        device.destroyDescriptorPool(descriptorPool->descriptorPool, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyFramebuffer(VulkanFramebuffer* framebuffer)
    {
        device.destroyFramebuffer(framebuffer->framebuffer, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroyFence(VulkanFence* fence)
    {
        device.destroyFence(fence->fence, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroySemaphore(VulkanSemaphore* semaphore)
    {
        device.destroySemaphore(semaphore->semaphore, allocationCallbacks);
    }

    void VulkanGraphicsDevice::DestroySampler(VulkanSampler* sampler)
    {
        device.destroySampler(sampler->sampler, allocationCallbacks);
    }
}
