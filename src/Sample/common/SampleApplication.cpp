#include "SampleApplication.h"

#include <Husky/Vulkan.h>
#include <Husky/Math/Math.h>
#include <Husky/FileStream.h>
#include <Husky/VectorTypes.h>
#include <Husky/SceneV1/Loader/glTFLoader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Light.h>
#include <Husky/SceneV1/BufferSource.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Sampler.h>

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

bool SampleApplication::Initialize(const Vector<String>& args)
{
#if _WIN32
    allocationCallbacks = allocator.GetAllocationCallbacks();
#endif

    auto [createInstanceResult, createdInstance] = CreateVulkanInstance();
    if (createInstanceResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    instance = createdInstance;

    auto [createDebugCallbackResult, createdDebugCallback] = CreateDebugCallback(instance);
    if(createDebugCallbackResult != vk::Result::eSuccess)
    {
        return false;
    }

    debugCallback = createdDebugCallback;

    auto [enumeratePhysicalDevicesResult, physicalDevices] = instance.enumeratePhysicalDevices();
    if (enumeratePhysicalDevicesResult != vk::Result::eSuccess || physicalDevices.empty())
    {
        // TODO
        return false;
    }

    physicalDevice = PhysicalDevice{ ChoosePhysicalDevice(physicalDevices), allocationCallbacks };

#if _WIN32
    std::tie(hInstance, hWnd) = CreateMainWindow(GetMainWindowTitle(), width, height);

    if (hWnd == nullptr)
    {
        // TODO
        return false;
    }

    auto [createSurfaceResult, createdSurface] = Surface::CreateWin32Surface(instance, hInstance, hWnd, allocationCallbacks);
    if (createSurfaceResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    surface = std::move(createdSurface);

    ShowWindow(hWnd, SW_SHOW);
#endif

#if __APPLE__
    if(view == nullptr)
    {
        return false;
    }

    auto [createSurfaceResult, createdSurface] = Surface::CreateMacOSSurface(instance, view);
    if (createSurfaceResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    surface = std::move(createdSurface);
#endif

    glTF::glTFParser glTFparser;

    //String rootDir{ "C:\\Development\\HuskyResources\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\" };

    //String rootDir{ "/Users/spo1ler/Development/HuskyEngine/res/gltf2/sponza/" };
    //String filename { "Sponza.gltf" };

    String rootDir{ "/Users/spo1ler/Development/HuskyEngine/res/gltf2/Box/" };
    String filename{ "Box.gltf" };

    FileStream fileStream{ rootDir + filename, FileOpenModes::Read };

    auto root = glTFparser.ParseJSON(&fileStream);

    SceneV1::Loader::glTFLoader loader{ rootDir, root };
    auto scenes = loader.LoadScenes();

    //forwardRenderer = MakeUnique<Render::ForwardRenderer>(&physicalDevice, surface, width, height);
    deferredRenderer = MakeUnique<Render::DeferredRenderer>(&physicalDevice, surface, width, height);

    //forwardRenderer->Initialize();
    deferredRenderer->Initialize();

    auto& scene = scenes[0];

    auto lightNode1 = MakeRef<SceneV1::Node>();
    lightNode1->SetName("ln1");
    auto light1 = MakeRef<SceneV1::Light>();

    light1->SetType(SceneV1::LightType::Spot);
    light1->SetRange(15);
    light1->SetIndex(0);
    light1->SetColor({1.0, 0.0, 0.0});
    light1->SetDirection({ 0.995037, -0.0995037, 0 });
    light1->SetSpotlightAngle(0.3);
    lightNode1->SetLight(light1);

    auto lightNode2 = MakeRef<SceneV1::Node>();
    lightNode2->SetName("ln2");
    auto light2 = MakeRef<SceneV1::Light>();

    light2->SetType(SceneV1::LightType::Spot);
    light2->SetRange(15);
    light2->SetIndex(1);
    light2->SetColor({ 0.0, 0.0, 1.0 });
    light2->SetDirection({ 0.995037, 0.0995037, 0 });
    light2->SetSpotlightAngle(0.3);

    lightNode2->SetLight(light2);

    auto lightNode3 = MakeRef<SceneV1::Node>();
    lightNode3->SetName("ln3");
    auto light3 = MakeRef<SceneV1::Light>();

    light3->SetType(SceneV1::LightType::Point);
    light3->SetRange(15);
    light3->SetIndex(2);

    lightNode3->SetLight(light3);

    SceneV1::TransformProperties light1Transform;
    light1Transform.translation = Vec3{ 0, 2, 0 };
    lightNode1->SetTransform(light1Transform);

    SceneV1::TransformProperties light2Transform;
    light2Transform.translation = Vec3{ 0, 2, 0 };
    lightNode2->SetTransform(light2Transform);

    SceneV1::TransformProperties light3Transform;
    light3Transform.translation = Vec3{ 0, 2, 0 };
    lightNode3->SetTransform(light3Transform);

    scene->AddNode(lightNode1);
    scene->AddNode(lightNode2);
    scene->AddNode(lightNode3);

    auto [prepareSceneResult, prepScene] = deferredRenderer->PrepareScene(scene);
    HUSKY_ASSERT(prepareSceneResult);
    preparedScene = std::move(prepScene);

    deferredRenderer->UpdateScene(preparedScene);

    return true;
}

bool SampleApplication::Deinitialize()
{
    DestroyDebugCallback(instance, debugCallback);

    vk::Optional<const vk::AllocationCallbacks> ac = nullptr;
    if(allocationCallbacks.has_value())
    {
        ac = *allocationCallbacks;
    }

    instance.destroy(ac);
    return true;
}

void SampleApplication::Run()
{
#if _WIN32
    MSG msg{};

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            deferredRenderer->DrawScene(preparedScene);
        }
    }
#endif
}

void SampleApplication::Process()
{
    deferredRenderer->DrawScene(preparedScene);
}

vk::ResultValue<vk::Instance> SampleApplication::CreateVulkanInstance()
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
    ci.setEnabledLayerCount((int32)validationLayers.size());
    ci.setPpEnabledLayerNames(validationLayers.data());
    ci.setEnabledExtensionCount((int32)requiredExtensions.size());
    ci.setPpEnabledExtensionNames(requiredExtensions.data());

    vk::Optional<const vk::AllocationCallbacks> ac = nullptr;
    if(allocationCallbacks.has_value())
    {
        ac = *allocationCallbacks;
    }

    return vk::createInstance(ci, ac);
}

vk::ResultValue<vk::DebugReportCallbackEXT> SampleApplication::CreateDebugCallback(
    vk::Instance& instance)
{
    VkDebugReportCallbackCreateInfoEXT ci;
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ci.pNext = nullptr;

    ci.flags =
          VK_DEBUG_REPORT_INFORMATION_BIT_EXT
        | VK_DEBUG_REPORT_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_ERROR_BIT_EXT
        | VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    ci.pfnCallback = StaticDebugCallback;
    ci.pUserData = static_cast<VulkanDebugDelegate*>(this);

    const VkAllocationCallbacks* callbacks = nullptr;
    if(allocationCallbacks.has_value())
    {
        callbacks = reinterpret_cast<const VkAllocationCallbacks*>(&*allocationCallbacks);
    }

    PFN_vkCreateDebugReportCallbackEXT procAddr = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
    VkDebugReportCallbackEXT callback;
    vk::Result result = static_cast<vk::Result>(procAddr(instance, &ci, callbacks, &callback));

    vk::DebugReportCallbackEXT debugReportCallback{ callback };
    return { result, debugReportCallback };
}

void SampleApplication::DestroyDebugCallback(
    vk::Instance& instance,
    vk::DebugReportCallbackEXT& callback)
{
    const VkAllocationCallbacks* callbacks = nullptr;
    if(allocationCallbacks.has_value())
    {
        callbacks = reinterpret_cast<const VkAllocationCallbacks*>(&*allocationCallbacks);
    }

    PFN_vkDestroyDebugReportCallbackEXT procAddr = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));
    procAddr(instance, callback, callbacks);
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
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
        return 0;
    }

    default:
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }
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

Vector<const char8*> SampleApplication::GetRequiredInstanceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;

    requiredExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    requiredExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if _WIN32
    requiredExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __APPLE__
    requiredExtensionNames.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

    return requiredExtensionNames;
}

Vector<const char8*> SampleApplication::GetValidationLayerNames() const
{
    return
    {
        "VK_LAYER_LUNARG_standard_validation",
        //"VK_LAYER_LUNARG_assistant_layer"
    };
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
