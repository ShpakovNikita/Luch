#include "SampleApplication.h"

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

#if HUSKY_USE_METAL
    #include <Husky/Metal/MetalPhysicalDevice.h>
    #include <Husky/Metal/MetalSurface.h>
#elif USE_VULKAN
    static_assert(false, "Vulkan is not ready");
#endif

using namespace Husky;
using namespace Husky::Graphics;

#if HUSKY_USE_METAL
using namespace Husky::Metal;
#endif

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

#if HUSKY_USE_METAL
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
    String rootDir{ "/Users/spo1ler/Development/HuskyEngine/res/gltf2/sponza/" };
    String filename { "Sponza.gltf" };
#endif

#if _WIN32
    String rootDir{ "C:\\Development\\HuskyResources\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\" };
    String filename{ "Sponza.gltf" };
#endif

    FileStream fileStream{ rootDir + filename, FileOpenModes::Read };

    auto root = glTFparser.ParseJSON(&fileStream);

    SceneV1::Loader::glTFLoader loader{ rootDir, root };
    auto scenes = loader.LoadScenes();

    deferredRenderer = MakeUnique<Render::DeferredRenderer2>(physicalDevice, surface, width, height);

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

