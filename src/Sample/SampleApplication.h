#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/Vulkan.h>
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
    // TODO init with uint32_max
    struct QueueIndices
    {
        Husky::uint32 computeQueueFamilyIndex;
        Husky::uint32 graphicsQueueFamilyIndex;
        Husky::uint32 presentQueueFamilyIndex;

        Husky::Vector<Husky::uint32> uniqueIndices;
    };

    struct QueueInfo
    {
        vk::Queue computeQueue;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        QueueIndices indices;
        Husky::Vector<vk::Queue> uniqueQueues;
    };

    struct SwapchainCreateInfo
    {
        Husky::int32 imageCount;
        Husky::int32 width;
        Husky::int32 height;
        vk::Format format;
        vk::ColorSpaceKHR colorSpace;
        vk::PresentModeKHR presentMode;
    };

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
    vk::ResultValue<vk::SurfaceKHR> CreateSurface(
        vk::Instance& instance,
        HINSTANCE hInstance,
        HWND hWnd,
        const vk::AllocationCallbacks& allocationCallbacks);
#endif

    QueueIndices ChooseDeviceQueues(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface);

    vk::ResultValue<vk::Device> CreateDevice(
        vk::PhysicalDevice& physicalDevice,
        const QueueIndices& queueIndices,
        const vk::AllocationCallbacks& allocationCallbacks);

    QueueInfo CreateQueueInfo(vk::Device& device, QueueIndices&& indices);

    CommandPoolCreateResult CreateCommandPools(
        vk::Device& device,
        const QueueInfo& info,
        const vk::AllocationCallbacks& allocationCallbacks);

    vk::ResultValue<SampleApplication::SwapchainCreateInfo> ChooseSwapchainCreateInfo(
        vk::PhysicalDevice& physicalDevice,
        vk::SurfaceKHR& surface);

    vk::ResultValue<vk::SwapchainKHR> CreateSwapchain(
        vk::Device& device,
        vk::SurfaceKHR& surface,
        const QueueIndices& queueIndices,
        const SwapchainCreateInfo& swapchainCreateInfo,
        const vk::AllocationCallbacks& callbacks);

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
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::SurfaceKHR surface;
    QueueInfo queueInfo;
    vk::CommandPool graphicsCommandPool;
    vk::CommandPool presentCommandPool;
    vk::CommandPool computeCommandPool;
    Husky::Vector<vk::CommandPool> uniqueCommandPools;
    vk::SwapchainKHR swapchain;
    
#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

    Husky::int32 width = 800;
    Husky::int32 height = 600;
};