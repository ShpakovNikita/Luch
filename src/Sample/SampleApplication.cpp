#include "SampleApplication.h"

#include <Husky/Vulkan.h>

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

    physicalDevice = ChoosePhysicalDevice(physicalDevices);

#if _WIN32
    std::tie(hInstance, hWnd) = CreateMainWindow(GetMainWindowTitle(), width, height);

    if (hWnd == nullptr)
    {
        // TODO
        return;
    }

    auto [createSurfaceResult, createdSurface] = CreateSurface(instance, hInstance, hWnd, allocationCallbacks);
    if (createSurfaceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }
#endif

    auto queueIndices = ChooseDeviceQueues(physicalDevice, surface);

    auto [createDeviceResult, createdDevice] = CreateDevice(physicalDevice, queueIndices, allocationCallbacks);
    if (createDeviceResult != vk::Result::eSuccess)
    {
        // TODO
        return;
    }

    device = createdDevice;
    queueInfo.graphicsQueue = device.getQueue(queueIndices.graphicsQueueFamilyIndex, 0);
    queueInfo.presentQueue = device.getQueue(queueIndices.presentQueueFamilyIndex, 0);
    queueInfo.computeQueue = device.getQueue(queueIndices.computeQueueFamilyIndex, 0);
    queueInfo.indices = queueIndices;

    auto createCommandPoolResult = CreateCommandPools(device, queueInfo, allocationCallbacks);

}

void SampleApplication::Deinitialize()
{
    device.waitIdle();
    device.destroyCommandPool(graphicsCommandPool, allocationCallbacks);
    device.destroyCommandPool(presentCommandPool, allocationCallbacks);
    device.destroyCommandPool(computeCommandPool, allocationCallbacks);
    device.destroy(allocationCallbacks);
    instance.destroy(allocationCallbacks);
}

void SampleApplication::Run()
{

}

vk::ResultValue<vk::Instance> SampleApplication::CreateVulkanInstance(vk::AllocationCallbacks& allocationCallbacks)
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

vk::ResultValue<vk::DebugReportCallbackEXT> SampleApplication::CreateDebugCallback(vk::Instance& instance, vk::AllocationCallbacks& allocationCallbacks)
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
        ShowWindow(window, SW_NORMAL);
        static_assert(sizeof(LONG_PTR) >= sizeof(VulkanDebugDelegate*));
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<VulkanDebugDelegate*>(this)));
    }

    return { hInstance, window };
}

vk::ResultValue<vk::SurfaceKHR> SampleApplication::CreateSurface(
    vk::Instance& instance,
    HINSTANCE hInstance,
    HWND hWnd,
    vk::AllocationCallbacks& allocationCallbacks)
{
    vk::Win32SurfaceCreateInfoKHR ci;
    ci.setHinstance(hInstance);
    ci.setHwnd(hWnd);
    return instance.createWin32SurfaceKHR(ci, allocationCallbacks);
}

#endif

SampleApplication::QueueIndices SampleApplication::ChooseDeviceQueues(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface)
{
    static float32 priorities[] = { 1.0f };
    auto queueProperties = physicalDevice.getQueueFamilyProperties();
    Vector<VkBool32> supportsPresent;
    supportsPresent.resize(queueProperties.size());

    for (uint32 i = 0; i < queueProperties.size(); i++)
    {
        auto& properties = queueProperties[i];
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);
    }

    QueueIndices indices;

    // Find compute queue
    for (uint32 i = 0; i < queueProperties.size(); i++)
    {
        auto& properties = queueProperties[i];

        if ((properties.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute)
        {
            indices.computeQueueFamilyIndex = i;
        }
    }

    // Try to find queue that support both graphics and present operation
    bool foundGraphicsAndPresent = false;
    for (uint32 i = 0; i < queueProperties.size(); i++)
    {
        auto& properties = queueProperties[i];

        if(supportsPresent[i] && (properties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsQueueFamilyIndex = i;
            indices.presentQueueFamilyIndex = i;

            foundGraphicsAndPresent = true;
            break;
        }
    }

    // Find separate queues that support graphics and present operation
    if (!foundGraphicsAndPresent)
    {
        for (uint32 i = 0; i < queueProperties.size(); i++)
        {
            auto& properties = queueProperties[i];
            
            if (supportsPresent[i])
            {
                indices.presentQueueFamilyIndex = i;
            }

            if ((properties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics)
            {
                indices.graphicsQueueFamilyIndex = i;
            }
        }
    }

    return indices;
}

vk::ResultValue<vk::Device> SampleApplication::CreateDevice(
    vk::PhysicalDevice & physicalDevice,
    const QueueIndices& queueIndices,
    vk::AllocationCallbacks& allocationCallbacks)
{
    static float32 queuePriorities[] = { 1.0 };
    auto requiredDeviceExtensionNames = GetRequiredDeviceExtensionNames();

    vk::DeviceQueueCreateInfo graphicsCi;
    graphicsCi.setQueueCount(1);
    graphicsCi.setQueueFamilyIndex(queueIndices.graphicsQueueFamilyIndex);
    graphicsCi.setPQueuePriorities(queuePriorities);

    vk::DeviceQueueCreateInfo presentCi;
    presentCi.setQueueCount(1);
    presentCi.setQueueFamilyIndex(queueIndices.presentQueueFamilyIndex);
    presentCi.setPQueuePriorities(queuePriorities);

    vk::DeviceQueueCreateInfo computeCi;
    computeCi.setQueueCount(1);
    computeCi.setQueueFamilyIndex(queueIndices.computeQueueFamilyIndex);
    computeCi.setPQueuePriorities(queuePriorities);

    Vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    if (queueIndices.computeQueueFamilyIndex == queueIndices.graphicsQueueFamilyIndex
     && queueIndices.graphicsQueueFamilyIndex == queueIndices.presentQueueFamilyIndex)
    {
        queueCreateInfos.reserve(1);
        queueCreateInfos.push_back(graphicsCi);
    }
    else
    {
        if (queueIndices.graphicsQueueFamilyIndex == queueIndices.presentQueueFamilyIndex)
        {
            queueCreateInfos.reserve(2);
            queueCreateInfos.push_back(graphicsCi);
            queueCreateInfos.push_back(computeCi);
        }
        else if(queueIndices.graphicsQueueFamilyIndex == queueIndices.computeQueueFamilyIndex)
        {
            queueCreateInfos.reserve(2);
            queueCreateInfos.push_back(graphicsCi);
            queueCreateInfos.push_back(presentCi);
        }
        else
        {
            queueCreateInfos.reserve(3);
            queueCreateInfos.push_back(graphicsCi);
            queueCreateInfos.push_back(presentCi);
            queueCreateInfos.push_back(computeCi);
        }
    }

    vk::DeviceCreateInfo ci;
    ci.setEnabledExtensionCount(requiredDeviceExtensionNames.size());
    ci.setPpEnabledExtensionNames(requiredDeviceExtensionNames.data());
    ci.setPpEnabledLayerNames(0); // device layers are deprecated
    ci.setQueueCreateInfoCount(queueCreateInfos.size());
    ci.setPQueueCreateInfos(queueCreateInfos.data());

    return physicalDevice.createDevice(ci, allocationCallbacks);
}

SampleApplication::CommandPoolCreateResult SampleApplication::CreateCommandPools(
    vk::Device & device,
    const QueueInfo & info,
    vk::AllocationCallbacks& allocationCallbacks)
{
    auto& queueIndices = info.indices;

    vk::CommandPoolCreateInfo graphicsCi;
    graphicsCi.setQueueFamilyIndex(queueIndices.graphicsQueueFamilyIndex);

    vk::CommandPoolCreateInfo presentCi;
    presentCi.setQueueFamilyIndex(queueIndices.presentQueueFamilyIndex);

    vk::CommandPoolCreateInfo computeCi;
    computeCi.setQueueFamilyIndex(queueIndices.computeQueueFamilyIndex);

    if (queueIndices.computeQueueFamilyIndex == queueIndices.graphicsQueueFamilyIndex
        && queueIndices.graphicsQueueFamilyIndex == queueIndices.presentQueueFamilyIndex)
    {
        auto result = device.createCommandPool(graphicsCi, allocationCallbacks);
        return { result, result, result };
    }
    else
    {
        if (queueIndices.graphicsQueueFamilyIndex == queueIndices.presentQueueFamilyIndex)
        {
            auto graphicsAndPresentPoolResult = device.createCommandPool(graphicsCi, allocationCallbacks);
            auto computePoolResult = device.createCommandPool(computeCi, allocationCallbacks);
            return { graphicsAndPresentPoolResult, graphicsAndPresentPoolResult, computePoolResult };
        }
        else if (queueIndices.graphicsQueueFamilyIndex == queueIndices.computeQueueFamilyIndex)
        {
            auto graphicsAndComputePoolResult = device.createCommandPool(graphicsCi, allocationCallbacks);
            auto presentPoolResult = device.createCommandPool(computeCi, allocationCallbacks);
            return { graphicsAndComputePoolResult, graphicsAndComputePoolResult, presentPoolResult };
        }
        else
        {
            auto graphicstPoolResult = device.createCommandPool(graphicsCi, allocationCallbacks);
            auto presentPoolResult = device.createCommandPool(presentCi, allocationCallbacks);
            auto computePoolResult = device.createCommandPool(computeCi, allocationCallbacks);
            return { graphicstPoolResult, presentPoolResult, computePoolResult };
        }
    }
}

Vector<const char8*> SampleApplication::GetRequiredInstanceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;

#if _WIN32
    requiredExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    requiredExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return requiredExtensionNames;
}

Husky::Vector<const Husky::char8*> SampleApplication::GetRequiredDeviceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;
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
