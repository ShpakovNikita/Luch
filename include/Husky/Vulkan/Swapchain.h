#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Device.h>

namespace Husky::Vulkan
{
    class Device;

    struct SwapchainCreateInfo
    {
        Husky::uint32 imageCount = 0;
        Husky::uint32 width = 0;
        Husky::uint32 height = 0;
        vk::Format format = vk::Format::eUndefined;
        vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    };

    class Swapchain
    {
        friend Device;
    public:
        static constexpr int MaxSwapchainLength = 3;

        Swapchain() = default;

        ~Swapchain();

        static VulkanResultValue<SwapchainCreateInfo> ChooseSwapchainCreateInfo(
            int32 width,
            int32 height,
            PhysicalDevice* physicalDevice,
            Surface* surface);
    private:
        struct SwapchainImage
        {
            vk::Image image;
            vk::ImageView imageView;
        };

        template<int32 ImageCount>
        Swapchain(
            Device* aDevice,
            vk::SwapchainKHR aSwapchain,
            SwapchainCreateInfo aCreateInfo,
            int32 aSwapchainImageCount,
            const Array<SwapchainImage, ImageCount>& aSwapchainImages)
            : device(aDevice)
            , swapchain(aSwapchain)
            , createInfo(aCreateInfo)
            , swapchainImageCount(aSwapchainImageCount)
        {
            static_assert(ImageCount <= MaxSwapchainLength);

            std::copy(aSwapchainImages.begin(), aSwapchainImages.end(), swapchainImages.begin());
        }

        Swapchain(
            Device* device,
            vk::SwapchainKHR swapchain,
            SwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            const Vector<SwapchainImage>& swapchainImages);

        SwapchainCreateInfo createInfo;
        Husky::Array<SwapchainImage, MaxSwapchainLength> swapchainImages;
        int32 swapchainImageCount;
        Device* device;
        vk::SwapchainKHR swapchain;
    };
}
