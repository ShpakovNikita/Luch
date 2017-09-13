#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class Device;
    class Surface;

    struct SwapchainCreateInfo
    {
        Husky::int32 imageCount = 0;
        Husky::int32 width = 0;
        Husky::int32 height = 0;
        vk::Format format = vk::Format::eUndefined;
        vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    };

    class Swapchain
    {
        friend class Device;
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

        Swapchain(
            Device* device,
            vk::SwapchainKHR swapchain,
            SwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            const Vector<SwapchainImage>& swapchainImages);

        SwapchainCreateInfo createInfo;
        Husky::Array<SwapchainImage, MaxSwapchainLength> swapchainImages;
        int32 swapchainImageCount;
        Device* device = nullptr;
        vk::SwapchainKHR swapchain;
    };
}
