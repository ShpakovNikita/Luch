#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/Vulkan/Fence.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>

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

        // TODO semaphore
        inline VulkanResultValue<int32> AcquireNextImage(Fence* fence, Optional<int64> timeoutNs = {})
        {
            uint64 timeoutValue;
            if (timeoutNs.has_value())
            {
                timeoutValue = timeoutNs.value();
            }
            else
            {
                timeoutValue = Limits<uint64>::max();
            }

            auto [acquireResult, index] = device->device.acquireNextImageKHR(swapchain, timeoutValue, nullptr, fence->GetFence());
            if (acquireResult != vk::Result::eSuccess)
            {
                return { acquireResult };
            }
            else
            {
                return { acquireResult, index };
            }
        }

        inline Format GetFormat() const { return createInfo.format; }
        inline ImageView* GetImageView(int index) { return &swapchainImages[index].imageView; }
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
