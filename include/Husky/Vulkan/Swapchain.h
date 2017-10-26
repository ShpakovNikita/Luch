#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class GraphicsDevice;
    class Surface;
    class Fence;
    class Semaphore;

    struct SwapchainCreateInfo
    {
        int32 imageCount = 0;
        int32 width = 0;
        int32 height = 0;
        int32 arrayLayers = 1;
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

        // TODO semaphores and fences
        VulkanResultValue<int32> AcquireNextImage(Fence* fence, Semaphore* semaphore, Optional<Timeout> timeout = {});

        inline SwapchainCreateInfo GetSwapchainCreateInfo() const { return createInfo; }
        inline Format GetFormat() const { return createInfo.format; }
        inline ImageView* GetImageView(int index) { return &swapchainImages[index].imageView; }
        inline vk::SwapchainKHR GetSwapchain() { return swapchain; }
    private:
        struct SwapchainImage
        {
            Image image;
            ImageView imageView;
        };

        Swapchain(
            GraphicsDevice* device,
            vk::SwapchainKHR swapchain,
            SwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            Vector<SwapchainImage>&& swapchainImages);

        void Destroy();

        SwapchainCreateInfo createInfo;
        // TODO use static vector
        Vector<SwapchainImage> swapchainImages;
        int32 swapchainImageCount;
        GraphicsDevice* device = nullptr;
        vk::SwapchainKHR swapchain;
    };
}
