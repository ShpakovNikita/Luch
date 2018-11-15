#include "SampleApplication.h"

#include <Luch/Math/Math.h>
#include <Luch/FileStream.h>
#include <Luch/VectorTypes.h>
#include <Luch/SceneV1/Loader/glTFLoader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/BufferSource.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Sampler.h>

#if LUCH_USE_METAL
    #include <Luch/Metal/MetalPhysicalDevice.h>
    #include <Luch/Metal/MetalSurface.h>
#elif USE_VULKAN
    static_assert(false, "Vulkan is not ready");
#endif

using namespace Luch;
using namespace Luch::Graphics;

#if LUCH_USE_METAL
using namespace Luch::Metal;
#endif

#ifdef _WIN32

static LRESULT CALLBACK StaticWindowProc(
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    using namespace Luch::Platform::Win32;

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

#if LUCH_USE_METAL
    auto [enumeratePhysicalDevicesResult, physicalDevices] = MetalPhysicalDevice::EnumeratePhysicalDevices();
    if (enumeratePhysicalDevicesResult != GraphicsResult::Success || physicalDevices.empty())
    {
        // TODO
        return false;
    }

    physicalDevice = physicalDevices.front();

    surface = MakeRef<MetalSurface>(view);
#endif

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

    glTF::glTFParser glTFparser;

#if __APPLE__
    String rootDir{ "/Users/spo1ler/Development/Luch/res/gltf2/sponza/" };
    String filename { "Sponza.gltf" };
#endif

#if _WIN32
    String rootDir{ "C:\\Development\\LuchResources\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\" };
    String filename{ "Sponza.gltf" };
#endif

    FileStream fileStream{ rootDir + filename, FileOpenModes::Read };

    auto root = glTFparser.ParseJSON(&fileStream);

    SceneV1::Loader::glTFLoader loader{ rootDir, root };
    scene = loader.LoadScene(0);

    deferredRenderer = MakeUnique<Render::Deferred::DeferredRenderer>(physicalDevice, surface, width, height);

    deferredRenderer->Initialize();

    auto cameraIt = std::find_if(
        scene->GetNodes().begin(),
        scene->GetNodes().end(),
        [](const auto& node) { return node->GetCamera() != nullptr; });

    LUCH_ASSERT(cameraIt != scene->GetNodes().end());
    camera = (*cameraIt)->GetCamera();

    deferredRenderer->PrepareScene(scene);
    deferredRenderer->UpdateScene(scene);

    return true;
}

bool SampleApplication::Deinitialize()
{
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
            deferredRenderer->DrawScene(scene);
        }
    }
#endif
}

void SampleApplication::Process()
{
    scene->Update();
    deferredRenderer->UpdateScene(scene);
    deferredRenderer->DrawScene(scene, camera);
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

std::tuple<HINSTANCE, HWND> SampleApplication::CreateMainWindow(const Luch::String& title, int32 width, int32 height)
{
    static const TCHAR* className = TEXT("MAIN_WINDOW");

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = StaticWindowProc;
    wc.cbWndExtra = sizeof(Luch::Platform::Win32::WndProcDelegate*);
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
        using namespace Luch::Platform::Win32;
        static_assert(sizeof(LONG_PTR) >= sizeof(WndProcDelegate*));
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<WndProcDelegate*>(this)));

        ShowWindow(window, SW_NORMAL);
    }

    return { hInstance, window };
}

#endif

