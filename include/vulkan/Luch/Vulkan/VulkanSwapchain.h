#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>
#include <Luch/RefPtr.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Vulkan/VulkanImage.h>
#include <Luch/Vulkan/VulkanImageView.h>

namespace Luch::Vulkan
{
    struct VulkanSwapchainImage
    {
        RefPtr<VulkanImage> image;
        RefPtr<VulkanImageView> imageView;
    };

    struct VulkanSwapchainCreateInfo
    {
        int32 imageCount = 0;
        int32 width = 0;
        int32 height = 0;
        int32 arrayLayers = 1;
        Format format = Format::Undefined;
        vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    };

    class VulkanSwapchain : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSwapchain(
            VulkanGraphicsDevice* device,
            vk::SwapchainKHR swapchain,
            VulkanSwapchainCreateInfo createInfo,
            int32 swapchainImageCount,
            Vector<VulkanSwapchainImage>&& swapchainImages);

        ~VulkanSwapchain() override;

        static VulkanResultValue<VulkanSwapchainCreateInfo> ChooseSwapchainCreateInfo(
            int32 width,
            int32 height,
            vk::PhysicalDevice vulkanPhysicalDevice,
            vk::SurfaceKHR vulkanSurface);

        // TODO semaphores and fences
        VulkanResultValue<int32> AcquireNextImage(VulkanFence* fence, VulkanSemaphore* semaphore, Optional<Timeout> timeout = {});

        inline VulkanSwapchainCreateInfo GetSwapchainCreateInfo() const { return createInfo; }
        inline Format GetFormat() const { return createInfo.format; }
        inline VulkanImageView* GetImageView(int index) { return swapchainImages[index].imageView; }
        inline vk::SwapchainKHR GetSwapchain() { return swapchain; }
    private:
        void Destroy();

        VulkanSwapchainCreateInfo createInfo;
        // TODO use static vector
        Vector<VulkanSwapchainImage> swapchainImages;
        int32 swapchainImageCount = 0;
        VulkanGraphicsDevice* device = nullptr;
        vk::SwapchainKHR swapchain;
    };
}
