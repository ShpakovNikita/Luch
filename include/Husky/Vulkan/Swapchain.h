#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Format.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class GraphicsDevice;
    class Surface;

    struct SwapchainCreateInfo
    {
        int32 imageCount = 0;
        int32 width = 0;
        int32 height = 0;
        Format format = Format::Undefined;
        vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    };

    class Swapchain
    {
        friend class GraphicsDevice;
    public:
        static constexpr int MaxSwapchainLength = 3;

        Swapchain() = default;

        Swapchain(Swapchain&& other);
        Swapchain& operator=(Swapchain&& other);

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
            GraphicsDevice* device,
            vk::SwapchainKHR swapchain,
            SwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            const Vector<SwapchainImage>& swapchainImages);

        void Destroy();

        SwapchainCreateInfo createInfo;
        Array<SwapchainImage, MaxSwapchainLength> swapchainImages;
        int32 swapchainImageCount;
        GraphicsDevice* device = nullptr;
        vk::SwapchainKHR swapchain;
    };
}
