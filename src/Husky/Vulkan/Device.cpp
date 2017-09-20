#include <Husky/Vulkan/Device.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>

namespace Husky::Vulkan
{
    Device::Device(PhysicalDevice* aPhysicalDevice, vk::Device aDevice, QueueInfo&& aQueueInfo, vk::AllocationCallbacks aAllocationCallbacks)
        : allocationCallbacks(aAllocationCallbacks)
        , queueInfo(std::move(aQueueInfo))
        , physicalDevice(aPhysicalDevice)
        , device(aDevice)
    {
    }

    Device::Device(Device && other)
        : physicalDevice(other.physicalDevice)
        , device(other.device)
        , allocationCallbacks(other.allocationCallbacks)
    {
        other.physicalDevice = nullptr;
        other.device = nullptr;
    }

    Device& Device::operator=(Device&& other)
    {
        physicalDevice = other.physicalDevice;
        device = other.device;
        allocationCallbacks = other.allocationCallbacks;

        other.physicalDevice = nullptr;
        other.device = nullptr;

        return *this;
    }

    Device::~Device()
    {
        if (device)
        {
            device.waitIdle();

            device.destroy(allocationCallbacks);
            device = nullptr;
        }
    }

    vk::Result Device::WaitIdle()
    {
        return device.waitIdle();
    }

    int32 Device::ChooseMemoryType(Husky::uint32 memoryTypeBits, vk::MemoryPropertyFlags memoryProperties)
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

    VulkanResultValue<Swapchain> Device::CreateSwapchain(const SwapchainCreateInfo & swapchainCreateInfo, Surface * surface)
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

    VulkanResultValue<vk::DeviceMemory> Device::AllocateMemory(
        int64 size,
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
        allocateInfo.setAllocationSize(size);
        allocateInfo.setMemoryTypeIndex(memoryTypeIndex);

        auto [result, vulkanMemory] = device.allocateMemory(allocateInfo, allocationCallbacks);
        if (result != vk::Result::eSuccess)
        {
            return { result, vulkanMemory };
        }
        else
        {
            device.freeMemory(vulkanMemory, allocationCallbacks);
            return { result };
        }
    }

    VulkanResultValue<CommandPool> Device::CreateCommandPool(QueueIndex queueIndex, bool transient, bool canReset)
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

    VulkanResultValue<Buffer> Device::CreateBuffer(int64 size, QueueIndex queueIndex, vk::BufferUsageFlags usage)
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

        auto [allocateMemoryResult, vulkanMemory] = AllocateMemory(size, memoryRequirements, vk::MemoryPropertyFlagBits::eHostVisible);
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

        return { createBufferResult, Buffer{this, vulkanBuffer, ci} };
    }

    void Device::DestroySwapchain(Swapchain * swapchain)
    {
        device.destroySwapchainKHR(swapchain->swapchain, allocationCallbacks);
    }

    void Device::DestroyCommandPool(CommandPool * commandPool)
    {
        device.destroyCommandPool(commandPool->commandPool, allocationCallbacks);
    }
}
