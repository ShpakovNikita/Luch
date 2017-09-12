#include <Husky/Vulkan/Device.h>
#include <Husky/Vulkan/Surface.h>

namespace Husky::Vulkan
{
    inline Device::Device(Device && other)
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
            
            auto swapchain = Swapchain{this, vulkanSwapchain, swapchainCreateInfo, swapchainImageCount, swapchainImages };
            return { result, std::move(swapchain) };
        }
        else
        {
            device.destroySwapchainKHR(vulkanSwapchain);
            return { result };
        }
    }

    Device::Device(PhysicalDevice* aPhysicalDevice, vk::Device aDevice, QueueInfo&& aQueueInfo, vk::AllocationCallbacks aAllocationCallbacks)
        : allocationCallbacks(aAllocationCallbacks)
        , queueInfo(std::move(aQueueInfo))
        , physicalDevice(aPhysicalDevice)
        , device(aDevice)
    {
    }
}
