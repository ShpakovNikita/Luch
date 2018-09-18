#pragma once

#include <Husky/Types.h>
#include <Husky/Format.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>
#include <Husky/Vulkan/VulkanImage.h>
#include <Husky/Vulkan/VulkanImageView.h>
#include <Husky/Vulkan/SwapchainCreateInfo.h>

namespace Husky::Vulkan
{
    struct VulkanSwapchainImage
    {
        RefPtr<VulkanImage> image;
        RefPtr<VulkanImageView> imageView;
    };

    class VulkanSwapchain : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSwapchain(
            VulkanGraphicsDevice* device,
            vk::SwapchainKHR swapchain,
            SwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            Vector<VulkanSwapchainImage>&& swapchainImages);

        ~VulkanSwapchain() override;

        static VulkanResultValue<SwapchainCreateInfo> ChooseSwapchainCreateInfo(
            int32 width,
            int32 height,
            VulkanPhysicalDevice* physicalDevice,
            VulkanSurface* surface);

        // TODO semaphores and fences
        VulkanResultValue<int32> AcquireNextImage(VulkanFence* fence, Semaphore* semaphore, Optional<Timeout> timeout = {});

        inline SwapchainCreateInfo GetSwapchainCreateInfo() const { return createInfo; }
        inline Format GetFormat() const { return createInfo.format; }
        inline VulkanImageView* GetImageView(int index) { return swapchainImages[index].imageView; }
        inline vk::SwapchainKHR GetSwapchain() { return swapchain; }
    private:
        void Destroy();

        SwapchainCreateInfo createInfo;
        // TODO use static vector
        Vector<VulkanSwapchainImage> swapchainImages;
        int32 swapchainImageCount = 0;
        VulkanGraphicsDevice* device = nullptr;
        vk::SwapchainKHR swapchain;
    };
}
