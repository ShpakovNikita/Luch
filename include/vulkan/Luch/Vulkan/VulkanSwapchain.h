#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>
#include <Luch/RefPtr.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Vulkan/VulkanImage.h>
#include <Luch/Vulkan/VulkanImageView.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/Swapchain.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    struct VulkanSwapchainImage
    {
        RefPtr<VulkanImage> image;
        RefPtr<VulkanImageView> imageView;
    };

    struct VulkanSwapchainCreateInfo : public SwapchainInfo
    {
        VulkanSwapchainCreateInfo() = default;
        VulkanSwapchainCreateInfo(const SwapchainInfo& si)
        {
            imageCount = si.imageCount;
            format = si.format;
            width = si.width;
            height = si.height;
        }

        int32 arrayLayers = 1;
        vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    };

    class VulkanSwapchain : public Swapchain
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

        static GraphicsResultValue<VulkanSwapchainCreateInfo> ChooseSwapchainCreateInfo(
            int32 width,
            int32 height,
            vk::PhysicalDevice vulkanPhysicalDevice,
            vk::SurfaceKHR vulkanSurface);

        const SwapchainInfo& GetInfo() const override
        {
            return createInfo;
        }

        // todo: remove if not needed, duplicates GetInfo()
        inline VulkanSwapchainCreateInfo GetSwapchainCreateInfo() const { return createInfo; }

        GraphicsResultValue<AcquiredTexture> GetNextAvailableTexture(Semaphore* semaphore) override;
        // TODO semaphores and fences
        GraphicsResultValue<int32> AcquireNextImage(VulkanFence* fence, VulkanSemaphore* semaphore, Optional<Timeout> timeout = {});

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
