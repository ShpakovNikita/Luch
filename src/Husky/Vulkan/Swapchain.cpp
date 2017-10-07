#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Math/Math.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>

namespace Husky::Vulkan
{
    // TODO use static vector

    VulkanResultValue<SwapchainCreateInfo> Swapchain::ChooseSwapchainCreateInfo(
        int32 width,
        int32 height,
        PhysicalDevice* physicalDevice,
        Surface* surface)
    {
        using namespace Husky::Math;

        SwapchainCreateInfo swapchainCreateInfo;

        auto vulkanSurface = surface->GetSurface();
        auto vulkanPhysicalDevice = physicalDevice->GetPhysicalDevice();

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
            swapchainCreateInfo.format = Format::R8G8B8A8Unorm;
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

    Swapchain::Swapchain(
        GraphicsDevice* aDevice,
        vk::SwapchainKHR aSwapchain,
        SwapchainCreateInfo aCreateInfo,
        int32 aSwapchainImageCount,
        const Vector<SwapchainImage>& aSwapchainImages)
        : device(aDevice)
        , swapchain(aSwapchain)
        , createInfo(aCreateInfo)
        , swapchainImageCount(aSwapchainImageCount)
    {
        // TODO ASSERT(aSwapchainImages.size() <= swapchainImages.size())
        std::copy(aSwapchainImages.begin(), aSwapchainImages.end(), swapchainImages.begin());
    }

    void Swapchain::Destroy()
    {
        if (device)
        {
            device->DestroySwapchain(this);
        }
    }

    Swapchain::Swapchain(Swapchain&& other)
        : device(other.device)
        , swapchain(other.swapchain)
        , createInfo(other.createInfo)
        , swapchainImageCount(other.swapchainImageCount)
        , swapchainImages(std::move(other.swapchainImages))
    {
        other.device = nullptr;
        other.swapchain = nullptr;
    }

    Swapchain & Swapchain::operator=(Swapchain && other)
    {
        Destroy();

        device = other.device;
        swapchain = other.swapchain;
        createInfo = other.createInfo;
        swapchainImageCount = other.swapchainImageCount;
        swapchainImages = std::move(other.swapchainImages);

        other.device = nullptr;
        other.swapchain = nullptr;

        return *this;
    }

    Swapchain::~Swapchain()
    {
        Destroy();
    }
}
