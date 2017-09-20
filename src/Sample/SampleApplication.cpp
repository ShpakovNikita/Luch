#include "SampleApplication.h"

#include <Husky/Vulkan.h>
#include <Husky/Math/Math.h>

#include <iostream>

using namespace Husky;
using namespace Husky::Vulkan;

static VkBool32 StaticDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64 object,
    size_t location,
    int32 messageCode,
    const char8 * pLayerPrefix,
    const char8 * pMessage,
    void* userData)
{
    return static_cast<VulkanDebugDelegate*>(userData)->DebugCallback(flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

#ifdef _WIN32

static LRESULT CALLBACK StaticWindowProc(
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    using namespace Husky::Platform::Win32;

    static_assert(sizeof(LPARAM) >= sizeof(WndProcDelegate*));

    auto additionalData = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    WndProcDelegate* wndProcDelegate = reinterpret_cast<WndProcDelegate*>(additionalData);

    if (wndProcDelegate != nullptr)
    {
        return wndProcDelegate->WndProc(hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
#endif

void SampleApplication::Initialize(const Vector<String>& args)
{
    allocationCallbacks = allocator.GetAllocationCallbacks();

    auto [createInstanceResult, createdInstance] = CreateVulkanInstance(allocationCallbacks);
    if (createInstanceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    instance = createdInstance;

#ifdef _DEBUG
    auto [createDebugCallbackResult, createdDebugCallback] = CreateDebugCallback(instance, allocationCallbacks);
    if(createDebugCallbackResult != vk::Result::eSuccess)
    {
        // TODO
    }

    debugCallback = createdDebugCallback;
#endif

    auto [enumeratePhysicalDevicesResult, physicalDevices] = instance.enumeratePhysicalDevices();
    if (enumeratePhysicalDevicesResult != vk::Result::eSuccess || physicalDevices.empty())
    {
        // TODO
        return;
    }

    physicalDevice = { ChoosePhysicalDevice(physicalDevices), allocationCallbacks };

#if _WIN32
    std::tie(hInstance, hWnd) = CreateMainWindow(GetMainWindowTitle(), width, height);

    if (hWnd == nullptr)
    {
        // TODO
        return;
    }

    auto [createSurfaceResult, createdSurface] = Surface::CreateWin32Surface(instance, hInstance, hWnd, allocationCallbacks);
    if (createSurfaceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    surface = std::move(createdSurface);
#endif

    auto [chooseQueuesResult, queueIndices] = physicalDevice.ChooseDeviceQueues(&surface);
    if (chooseQueuesResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    auto [createDeviceResult, createdDevice] = physicalDevice.CreateDevice(std::move(queueIndices), GetRequiredDeviceExtensionNames());
    if (createDeviceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    device = std::move(createdDevice);

    auto& indices = device.GetQueueIndices();

    auto [createdGraphicsCommandPoolResult, createdGraphicsCommandPool] = device.CreateCommandPool(indices.graphicsQueueFamilyIndex, false, false);
    if (createdGraphicsCommandPoolResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    auto[createdPresentCommandPoolResult, createdPresentCommandPool] = device.CreateCommandPool(indices.presentQueueFamilyIndex, true, false);
    if (createdPresentCommandPoolResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    auto[createdComputeCommandPoolResult, createdComputeCommandPool] = device.CreateCommandPool(indices.computeQueueFamilyIndex, false, true);
    if (createdComputeCommandPoolResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    graphicsCommandPool = std::move(createdGraphicsCommandPool);
    presentCommandPool = std::move(createdPresentCommandPool);
    computeCommandPool = std::move(createdComputeCommandPool);

    auto [swapchainChooseCreateInfoResult, swapchainCreateInfo] = Swapchain::ChooseSwapchainCreateInfo(width, height, &physicalDevice, &surface);
    if (swapchainChooseCreateInfoResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    auto [createSwapchainResult, createdSwapchain] = device.CreateSwapchain(swapchainCreateInfo, &surface);
    if (createSwapchainResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }


}

void SampleApplication::Deinitialize()
{
    DestroyDebugCallback(instance, debugCallback, allocationCallbacks);
    instance.destroy(allocationCallbacks);
}

void SampleApplication::Run()
{

}

vk::ResultValue<vk::Instance> SampleApplication::CreateVulkanInstance(const vk::AllocationCallbacks& allocationCallbacks)
{
    auto requiredExtensions = GetRequiredInstanceExtensionNames();
    auto validationLayers = GetValidationLayerNames();

    auto applicationName = GetApplicationName();

    vk::ApplicationInfo applicationInfo;
    applicationInfo.setApiVersion(VK_MAKE_VERSION(1, 0, 56));
    applicationInfo.setApplicationVersion(VK_MAKE_VERSION(0, 1, 0));
    applicationInfo.setPApplicationName(applicationName.c_str());
    applicationInfo.setEngineVersion(VK_MAKE_VERSION(0, 1, 0));
    applicationInfo.setPEngineName("Husky Engine");

    vk::InstanceCreateInfo ci;
    ci.setPApplicationInfo(&applicationInfo);
    ci.setEnabledLayerCount(validationLayers.size());
    ci.setPpEnabledLayerNames(validationLayers.data());
    ci.setEnabledExtensionCount(requiredExtensions.size());
    ci.setPpEnabledExtensionNames(requiredExtensions.data());

    return vk::createInstance(ci, allocationCallbacks);
}

vk::ResultValue<vk::DebugReportCallbackEXT> SampleApplication::CreateDebugCallback(
    vk::Instance& instance,
    const vk::AllocationCallbacks& allocationCallbacks)
{
    VkDebugReportCallbackCreateInfoEXT ci;
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ci.pNext = nullptr;
    ci.pfnCallback = StaticDebugCallback;
    ci.pUserData = static_cast<VulkanDebugDelegate*>(this);

    const VkAllocationCallbacks& callbacks = allocationCallbacks;

    PFN_vkCreateDebugReportCallbackEXT procAddr = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
    VkDebugReportCallbackEXT callback;
    vk::Result result = static_cast<vk::Result>(procAddr(instance, &ci, &callbacks, &callback));

    vk::DebugReportCallbackEXT debugReportCallback{ callback };
    return { result, debugReportCallback };
}

void SampleApplication::DestroyDebugCallback(
    vk::Instance& instance,
    vk::DebugReportCallbackEXT& callback,
    const vk::AllocationCallbacks& allocationCallbacks)
{
    const VkAllocationCallbacks& callbacks = allocationCallbacks;
    PFN_vkDestroyDebugReportCallbackEXT procAddr = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));
    procAddr(instance, callback, &callbacks);
}

vk::PhysicalDevice SampleApplication::ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices)
{
    return devices[0];
}

#ifdef _WIN32

LRESULT SampleApplication::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

std::tuple<HINSTANCE, HWND> SampleApplication::CreateMainWindow(const Husky::String& title, int32 width, int32 height)
{
    static const TCHAR* className = TEXT("MAIN_WINDOW");

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = StaticWindowProc;
    wc.cbWndExtra = sizeof(Husky::Platform::Win32::WndProcDelegate*);
    wc.lpszClassName = className;

    RegisterClass(&wc);

    // TODO string conversion
    HWND window = CreateWindowEx(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (window)
    {
        using namespace Husky::Platform::Win32;
        static_assert(sizeof(LONG_PTR) >= sizeof(WndProcDelegate*));
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<WndProcDelegate*>(this)));

        ShowWindow(window, SW_NORMAL);
    }

    return { hInstance, window };
}

#endif

//vk::ResultValue<vk::Image> SampleApplication::CreateDepthStencilBufferForSwapchain(
//    vk::Device& device,
//    vk::Format format,
//    uint32 graphicsQueueFamilyIndex,
//    const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
//    const SwapchainCreateInfo& swapchainCreateInfo,
//    const vk::AllocationCallbacks& allocationCallbacks)
//{
//    vk::ImageCreateInfo ci;
//    ci.setFormat(format);
//    ci.setArrayLayers(1);
//    ci.setImageType(vk::ImageType::e2D);
//    ci.setExtent({ swapchainCreateInfo.width, swapchainCreateInfo.height, 1 });
//    ci.setInitialLayout(vk::ImageLayout::eUndefined);
//    ci.setMipLevels(1);
//    ci.setQueueFamilyIndexCount(1);
//    ci.setPQueueFamilyIndices(&graphicsQueueFamilyIndex);
//    ci.setSamples(vk::SampleCountFlagBits::e1);
//    ci.setSharingMode(vk::SharingMode::eExclusive);
//    ci.setTiling(vk::ImageTiling::eOptimal);
//    ci.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);
//
//    auto [createImageResult, createdImage] = device.createImage(ci, allocationCallbacks);
//    if (createImageResult != vk::Result::eSuccess)
//    {
//        return { createImageResult, createdImage };
//    }
//
//    auto memoryRequirements = device.getImageMemoryRequirements(createdImage);
//    auto memoryType = ChooseMemoryType(physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//
//    vk::MemoryAllocateInfo memoryAllocInfo;
//    memoryAllocInfo.setAllocationSize(memoryRequirements.size);
//    memoryAllocInfo.setMemoryTypeIndex(memoryType);
//
//    auto [allocateResult, allocatedMemory] = device.allocateMemory(memoryAllocInfo);
//    if (allocateResult != vk::Result::eSuccess)
//    {
//        return { allocateResult, createdImage };
//    }
//
//    auto bindResult = device.bindImageMemory(createdImage, allocatedMemory, 0);
//    if (bindResult != vk::Result::eSuccess)
//    {
//        return { bindResult, createdImage };
//    }
//
//    return { vk::Result::eSuccess, createdImage };
//}

vk::ResultValue<vk::ImageView> SampleApplication::CreateDepthStencilBufferViewForSwapchain(
    vk::Device& device,
    vk::Format format,
    vk::Image& depthStencilBuffer,
    const vk::AllocationCallbacks& allocationCallbacks)
{
    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
    subresourceRange.setLayerCount(1);
    subresourceRange.setLevelCount(1);

    vk::ImageViewCreateInfo ci;
    ci.setImage(depthStencilBuffer);
    ci.setFormat(format);
    ci.setViewType(vk::ImageViewType::e2D);
    ci.setSubresourceRange(subresourceRange);

    return device.createImageView(ci, allocationCallbacks);
}

Vector<const char8*> SampleApplication::GetRequiredInstanceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;

    requiredExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    requiredExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if _WIN32
    requiredExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    return requiredExtensionNames;
}

Husky::Vector<const Husky::char8*> SampleApplication::GetRequiredDeviceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;

    requiredExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return requiredExtensionNames;
}

Vector<const char8*> SampleApplication::GetValidationLayerNames() const
{
    return {"VK_LAYER_LUNARG_standard_validation"};
}

VkBool32 SampleApplication::DebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char * pLayerPrefix,
    const char * pMessage)
{
    std::cout << pLayerPrefix << " | " << pMessage << "\n";
    return VK_TRUE;
}
