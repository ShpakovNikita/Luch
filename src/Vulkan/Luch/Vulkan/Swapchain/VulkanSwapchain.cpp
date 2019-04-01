#include <Luch/Vulkan/Swapchain/VulkanSwapchain.h>
#include <Luch/Math/Math.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/Common/VulkanFormat.h>
#include <Luch/Vulkan/VulkanPhysicalDevice.h>
#include <Luch/Vulkan/VulkanSurface.h>
#include <Luch/Vulkan/Sync/VulkanFence.h>
#include <Luch/Vulkan/Sync/VulkanSemaphore.h>

namespace Luch::Vulkan
{
    VulkanSwapchain::VulkanSwapchain(
        VulkanGraphicsDevice* aDevice,
        vk::SwapchainKHR aSwapchain,
        VulkanSwapchainCreateInfo aCreateInfo,
        int32 aSwapchainImageCount,
        Vector<VulkanSwapchainImage>&& aSwapchainImages)
        : Swapchain(aDevice)
        , device(aDevice)
        , swapchain(aSwapchain)
        , createInfo(aCreateInfo)
        , swapchainImageCount(aSwapchainImageCount)
        , swapchainImages(move(aSwapchainImages))
    {
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        Destroy();
    }

    void VulkanSwapchain::Destroy()
    {
        if (device)
        {
            device->DestroySwapchain(this);
        }
    }

    GraphicsResultRefPtr<SwapchainTexture> VulkanSwapchain::GetNextAvailableTexture()
    {
        // todo: implement
        return {GraphicsResult::Unsupported};
    }

    GraphicsResultValue<int32> VulkanSwapchain::AcquireNextImage(
        VulkanFence* fence,
        VulkanSemaphore* semaphore,
        Optional<Timeout> timeout)
    {
        LUCH_ASSERT(fence || semaphore);

        auto vulkanFence = fence ? fence->GetFence() : nullptr;
        auto vulkanSemaphore = semaphore ? semaphore->GetSemaphore() : nullptr;

        auto[acquireResult, index] = device->device.acquireNextImageKHR(
            swapchain,
            ToVulkanTimeout(timeout),
            vulkanSemaphore,
            vulkanFence);

        if (acquireResult != vk::Result::eSuccess)
        {
            return { acquireResult };
        }
        else
        {
            return { acquireResult, (int32)index };
        }
    }

    // TODO use static vector

    GraphicsResultValue<VulkanSwapchainCreateInfo> VulkanSwapchain::ChooseSwapchainCreateInfo(
        int32 width,
        int32 height,
        vk::PhysicalDevice vulkanPhysicalDevice,
        vk::SurfaceKHR vulkanSurface)
    {
        using namespace Luch::Math;

        VulkanSwapchainCreateInfo swapchainCreateInfo;

        auto [getSurfaceCapabilitiesResult, surfaceCapabilities] = vulkanPhysicalDevice.getSurfaceCapabilitiesKHR(vulkanSurface);
        if (getSurfaceCapabilitiesResult != vk::Result::eSuccess)
        {
            return { getSurfaceCapabilitiesResult, swapchainCreateInfo };
        }

        auto [getSurfaceFormatsResult, surfaceFormats] = vulkanPhysicalDevice.getSurfaceFormatsKHR(vulkanSurface);
        if (getSurfaceCapabilitiesResult != vk::Result::eSuccess)
        {
            return { getSurfaceFormatsResult, swapchainCreateInfo };
        }

        if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
        {
            swapchainCreateInfo.format = Format::RGBA8Unorm;
            swapchainCreateInfo.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        }
        else
        {
            swapchainCreateInfo.format = FromVulkanFormat(surfaceFormats[0].format);
            swapchainCreateInfo.colorSpace = surfaceFormats[0].colorSpace;
        }

        if (surfaceCapabilities.currentExtent.width == Limits<uint32>::max())
        {
            swapchainCreateInfo.width = Clamp(
                uint32(width),
                surfaceCapabilities.minImageExtent.width,
                surfaceCapabilities.maxImageExtent.width);

            swapchainCreateInfo.height = Clamp(
                uint32(height),
                surfaceCapabilities.minImageExtent.height,
                surfaceCapabilities.maxImageExtent.height);
        }
        else
        {
            swapchainCreateInfo.width = surfaceCapabilities.currentExtent.width;
            swapchainCreateInfo.height = surfaceCapabilities.currentExtent.height;
        }

        swapchainCreateInfo.presentMode = vk::PresentModeKHR::eFifo;
        swapchainCreateInfo.imageCount = std::min(3u, surfaceCapabilities.maxImageCount);

        return { vk::Result::eSuccess, swapchainCreateInfo };
    }
}
