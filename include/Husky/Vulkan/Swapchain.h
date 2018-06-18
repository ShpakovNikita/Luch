#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Format.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/SwapchainCreateInfo.h>

namespace Husky::Vulkan
{
    class PhysicalDevice;
    class GraphicsDevice;
    class Surface;
    class Fence;
    class Semaphore;

    struct SwapchainImage
    {
        RefPtr<Image> image;
        RefPtr<ImageView> imageView;
    };

    class Swapchain : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Swapchain(
            GraphicsDevice* device,
            vk::SwapchainKHR swapchain,
            SwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            Vector<SwapchainImage>&& swapchainImages);

        Swapchain(Swapchain&& other) = delete;
        Swapchain(const Swapchain& other) = delete;
        Swapchain& operator=(const Swapchain& other) = delete;
        Swapchain& operator=(Swapchain&& other) = delete;

        ~Swapchain() override;

        static VulkanResultValue<SwapchainCreateInfo> ChooseSwapchainCreateInfo(
            int32 width,
            int32 height,
            PhysicalDevice* physicalDevice,
            Surface* surface);

        // TODO semaphores and fences
        VulkanResultValue<int32> AcquireNextImage(Fence* fence, Semaphore* semaphore, Optional<Timeout> timeout = {});

        inline SwapchainCreateInfo GetSwapchainCreateInfo() const { return createInfo; }
        inline Format GetFormat() const { return createInfo.format; }
        inline ImageView* GetImageView(int index) { return swapchainImages[index].imageView.Get(); }
        inline vk::SwapchainKHR GetSwapchain() { return swapchain; }
    private:
        void Destroy();

        SwapchainCreateInfo createInfo;
        // TODO use static vector
        Vector<SwapchainImage> swapchainImages;
        int32 swapchainImageCount = 0;
        GraphicsDevice* device = nullptr;
        vk::SwapchainKHR swapchain;
    };
}
