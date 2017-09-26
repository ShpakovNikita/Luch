#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>
#include <Husky/Vulkan/ShaderCompiler.h>

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

    GraphicsDevice::GraphicsDevice(GraphicsDevice&& other)
        : physicalDevice(other.physicalDevice)
        , device(other.device)
        , queueInfo(std::move(other.queueInfo))
        , allocationCallbacks(other.allocationCallbacks)
    {
        other.physicalDevice = nullptr;
        other.device = nullptr;
    }

    GraphicsDevice& GraphicsDevice::operator=(GraphicsDevice&& other)
    {
        physicalDevice = other.physicalDevice;
        device = other.device;
        queueInfo = std::move(other.queueInfo);
        allocationCallbacks = other.allocationCallbacks;

        other.physicalDevice = nullptr;
        other.device = nullptr;

        return *this;
    }

    GraphicsDevice::~GraphicsDevice()
    {
        if (device)
        {
            device.waitIdle();

            device.destroy(allocationCallbacks);
            device = nullptr;
        }
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

    VulkanResultValue<Swapchain> GraphicsDevice::CreateSwapchain(const SwapchainCreateInfo & swapchainCreateInfo, Surface * surface)
    {
        vk::SwapchainCreateInfoKHR ci;
        ci.setImageColorSpace(swapchainCreateInfo.colorSpace);
        ci.setImageFormat(swapchainCreateInfo.format);
        ci.setMinImageCount(swapchainCreateInfo.imageCount);
        ci.setPresentMode(swapchainCreateInfo.presentMode);
        ci.setImageArrayLayers(1);
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

            Vector<Swapchain::SwapchainImage> swapchainImages;
            swapchainImages.resize(swapchainImageCount);

            for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
            {
                auto image = images[i];

                vk::ImageSubresourceRange subresourceRange;
                subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
                subresourceRange.setLayerCount(1);
                subresourceRange.setLevelCount(1);

                vk::ImageViewCreateInfo ci;
                ci.setImage(image);
                ci.setViewType(vk::ImageViewType::e2D);
                ci.setFormat(swapchainCreateInfo.format);
                ci.setSubresourceRange(subresourceRange);
                auto [imageViewCreateResult, imageView] = device.createImageView(ci, allocationCallbacks);
                if (imageViewCreateResult != vk::Result::eSuccess)
                {
                    // TODO
                    return { imageViewCreateResult };
                }

                swapchainImages[i] = { image, imageView };
            }
            
            return { result, Swapchain{ this, vulkanSwapchain, swapchainCreateInfo, swapchainImageCount, swapchainImages } };
        }
        else
        {
            device.destroySwapchainKHR(vulkanSwapchain);
            return { result };
        }
    }

    VulkanResultValue<CommandPool> GraphicsDevice::CreateCommandPool(QueueIndex queueIndex, bool transient, bool canReset)
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
            return { result, CommandPool{ this, vulkanCommandPool } };
        }
    }

    VulkanResultValue<Buffer> GraphicsDevice::CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage)
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

        auto [allocateMemoryResult, vulkanMemory] = AllocateMemory(memoryRequirements, vk::MemoryPropertyFlagBits::eHostVisible);
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

        return { createBufferResult, Buffer{this, vulkanBuffer, vulkanMemory, ci} };
    }

    VulkanResultValue<Image> GraphicsDevice::CreateImage(const vk::ImageCreateInfo& imageCreateInfo)
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

        return { vk::Result::eSuccess, Image{this, vulkanImage, vulkanMemory, imageCreateInfo } };
    }

    VulkanResultValue<ImageView> GraphicsDevice::CreateImageView(Image* image, vk::ImageViewCreateInfo& imageViewCreateInfo)
    {
        imageViewCreateInfo.setImage(image->GetImage());
        auto[createImageViewResult, vulkanImageView] = device.createImageView(imageViewCreateInfo, allocationCallbacks);
        if (createImageViewResult != vk::Result::eSuccess)
        {
            device.destroyImageView(vulkanImageView, allocationCallbacks);
            return { createImageViewResult };
        }
        
        return { createImageViewResult, ImageView{this, vulkanImageView} };
    }

    VulkanResultValue<ImageView> GraphicsDevice::CreateImageView(Image * image)
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
                if((imageCi.flags & vk::ImageCreateFlagBits::eCubeCompatible) == vk::ImageCreateFlagBits::eCubeCompatible)
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

        return CreateImageView(image, ci);
    }

    VulkanResultValue<ShaderModule> GraphicsDevice::CompileShaderModule(ShaderCompiler* compiler, char8* sourceCode, int64 sourceCodeSize)
    {
        return VulkanResultValue<ShaderModule>();
    }

    VulkanResultValue<PipelineCache> GraphicsDevice::CreatePipelineCache()
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
            return { createResult, PipelineCache{this, vulkanPipelineCache} };
        }
    }

    VulkanResultValue<Pipeline> GraphicsDevice::CreateGraphicsPipeline(
        const GraphicsPipelineCreateInfo & graphicsPipelineCreateInfo,
        PipelineCache* pipelineCache)
    {
        vk::GraphicsPipelineCreateInfo ci;

        vk::PipelineCache cache = pipelineCache ? pipelineCache->pipelineCache : nullptr;

        auto[createResult, vulkanPipeline] = device.createGraphicsPipeline(cache, ci, allocationCallbacks);
        if (createResult != vk::Result::eSuccess)
        {
            device.destroyPipeline(vulkanPipeline, allocationCallbacks);
            return { createResult };
        }
        else
        {
            return { createResult, Pipeline{this, vulkanPipeline} };
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

    void GraphicsDevice::DestroyImage(Image* image)
    {
        device.freeMemory(image->memory, allocationCallbacks);
        device.destroyImage(image->image, allocationCallbacks);
    }

    void GraphicsDevice::DestroyImageView(ImageView * imageView)
    {
        device.destroyImageView(imageView->imageView, allocationCallbacks);
    }

    void GraphicsDevice::DestroyPipeline(Pipeline * pipeline)
    {
        device.destroyPipeline(pipeline->pipeline, allocationCallbacks);
    }

    void GraphicsDevice::DestroyPipelineCache(PipelineCache * pipelineCache)
    {
        device.destroyPipelineCache(pipelineCache->pipelineCache, allocationCallbacks);
    }

    void GraphicsDevice::DestroyShaderModule(ShaderModule * module)
    {
        device.destroyShaderModule(module->module, allocationCallbacks);
    }
}
