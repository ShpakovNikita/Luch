#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>
#include <Husky/Vulkan/Device.h>
#include "VulkanAllocator.h"

#ifdef _WIN32
#include <Husky/Platform/Win32/WndProcDelegate.h>
#endif

class SampleApplication
    : public Husky::BaseApplication
    , private Husky::Vulkan::VulkanDebugDelegate
#ifdef _WIN32
    , private Husky::Platform::Win32::WndProcDelegate
#endif
{
public:
    SampleApplication() = default;
    void Initialize(const Husky::Vector<Husky::String>& args) override;
    void Deinitialize() override;
    void Run() override;

    const Husky::String& GetApplicationName() const
    {
        static Husky::String applicationName = "Sample";
        return applicationName;
    }

    const Husky::Version& GetApplicationVersion() const
    {
        static Husky::Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }

    const Husky::String& GetMainWindowTitle() const
    {
        static Husky::String windowTitle = "Sample";
        return windowTitle;
    }
private:

    struct CommandPoolCreateResult
    {
        vk::ResultValue<vk::CommandPool> graphicsCommandPool;
        vk::ResultValue<vk::CommandPool> presentCommandPool;
        vk::ResultValue<vk::CommandPool> computeCommandPool;

        Husky::Vector<vk::CommandPool> uniqueCommandPools;
    };

    vk::ResultValue<vk::Instance> CreateVulkanInstance(const vk::AllocationCallbacks& allocationCallbacks);
    vk::ResultValue<vk::DebugReportCallbackEXT> CreateDebugCallback(vk::Instance& instance, const vk::AllocationCallbacks& allocationCallbacks);
    void DestroyDebugCallback(vk::Instance& instance, vk::DebugReportCallbackEXT& callback, const vk::AllocationCallbacks& allocationCallbacks);

    vk::PhysicalDevice ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices);

#ifdef _WIN32
    LRESULT WndProc(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    ) override;

    std::tuple<HINSTANCE, HWND> CreateMainWindow(const Husky::String& title, Husky::int32 width, Husky::int32 height);
#endif

    CommandPoolCreateResult CreateCommandPools(
        vk::Device& device,
        const QueueInfo& info,
        const vk::AllocationCallbacks& allocationCallbacks);

    vk::ResultValue<SampleApplication::SwapchainCreateInfo> ChooseSwapchainCreateInfo(
        vk::PhysicalDevice& physicalDevice,
        vk::SurfaceKHR& surface);

    vk::ResultValue<vk::Image> CreateDepthStencilBufferForSwapchain(
        vk::Device& device,
        vk::Format format,
        Husky::uint32 graphicsQueueFamilyIndex,
        const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
        const SwapchainCreateInfo& swapchainCreateInfo,
        const vk::AllocationCallbacks& allocationCallbacks
    );

    vk::ResultValue<vk::ImageView> CreateDepthStencilBufferViewForSwapchain(
        vk::Device& device,
        vk::Format format,
        vk::Image& depthStencilBuffer,
        const vk::AllocationCallbacks& allocationCallbacks
    );

    Husky::int32 ChooseMemoryType(
        const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
        Husky::uint32 memoryTypeBits,
        vk::MemoryPropertyFlags memoryProperties);

    Husky::Vector<const Husky::char8*> GetRequiredInstanceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetRequiredDeviceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetValidationLayerNames() const;

    vk::Bool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char * pLayerPrefix,
        const char * pMessage) override;

    VulkanAllocator allocator;

    vk::AllocationCallbacks allocationCallbacks;
    vk::Instance instance;
    vk::DebugReportCallbackEXT debugCallback;
    Husky::Vulkan::PhysicalDevice physicalDevice;
    Husky::Vulkan::Surface surface;
    Husky::Vulkan::Device device;


#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

    Husky::int32 width = 800;
    Husky::int32 height = 600;
};