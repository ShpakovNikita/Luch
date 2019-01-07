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

#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Render/SceneRenderer.h>
#include <algorithm>

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

    String rootDir{ "Data/gltf2/sponza/" };
    String filename { "Sponza.gltf" };

    FileStream fileStream{ rootDir + filename, FileOpenModes::Read };

    auto root = glTFparser.ParseJSON(&fileStream);

    SceneV1::Loader::glTFLoader loader{ rootDir, root };
    scene = loader.LoadScene(0);

    auto cameraIt = std::find_if(
        scene->GetNodes().begin(),
        scene->GetNodes().end(),
        [](const auto& node) { return node->GetCamera() != nullptr; });

    LUCH_ASSERT(cameraIt != scene->GetNodes().end());
    cameraNode = *cameraIt;

    wasdController.SetNode(cameraNode);
    mouseController.SetNode(cameraNode);

    context = MakeShared<Render::RenderContext>();

    auto [createDeviceResult, createdDevice] = physicalDevice->CreateGraphicsDevice();
    if(createDeviceResult != GraphicsResult::Success)
    {
        return false;
    }

    context->device = std::move(createdDevice);

    auto [createCommandQueueResult, createdCommandQueue] = context->device->CreateCommandQueue();
    if(createCommandQueueResult != GraphicsResult::Success)
    {
        return false;
    }

    context->commandQueue = std::move(createdCommandQueue);

    SwapchainInfo swapchainInfo;
    swapchainInfo.format = Format::B8G8R8A8Unorm_sRGB;
    swapchainInfo.imageCount = 1;
    swapchainInfo.width = width;
    swapchainInfo.height = height;

    auto [createSwapchainResult, createdSwapchain] = context->device->CreateSwapchain(swapchainInfo, surface);
    if(createSwapchainResult != GraphicsResult::Success)
    {
        return false;
    }

    context->swapchain = std::move(createdSwapchain);

    renderer = MakeUnique<Render::SceneRenderer>(scene);

    auto rendererInitialized = renderer->Initialize(context);
    if(!rendererInitialized)
    {
        return false;
    }

    for(int32 axis = WASDNodeController::XAxis; axis <= WASDNodeController::ZAxis; axis++)
    {
        wasdController.SetSpeed(axis, WASDNodeController::Negative, 2.5);
        wasdController.SetSpeed(axis, WASDNodeController::Positive, 2.5);
    }

    return true;
}

bool SampleApplication::Deinitialize()
{
    auto rendererDeinitialized = renderer->Deinitialize();
    if(!rendererDeinitialized)
    {
        return false;
    }

    return true;
}

bool SampleApplication::ShouldQuit()
{
    return shouldQuit;
}

void SampleApplication::Process()
{
    mouseController.Tick();
    wasdController.Tick(16.0f / 1000.0f);
    scene->Update();

    renderer->BeginRender();
    renderer->PrepareScene();
    renderer->UpdateScene();
    renderer->DrawScene(cameraNode);
    renderer->EndRender();

    context->commandQueue->Present(0, context->swapchain);
}

void SampleApplication::HandleEvent(const SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        HandleKeyboardEvent(event);
        break;
    case SDL_MOUSEMOTION:
        HandleMouseMotionEvent(event);
        break;
    case SDL_QUIT:
        shouldQuit = true;
        break;
    default:
        return;
    }
}

void SampleApplication::HandleKeyboardEvent(const SDL_Event& event)
{
    Optional<bool> moving;

    if(event.type == SDL_KEYDOWN)
    {
        moving = true;
    }

    if(event.type == SDL_KEYUP)
    {
        moving = false;
    }

    Optional<int32> axis;
    Optional<int32> direction;

    switch(event.key.keysym.scancode)
    {
    case SDL_SCANCODE_W:
        axis = WASDNodeController::XAxis;
        direction = WASDNodeController::Positive;
        break;
    case SDL_SCANCODE_S:
        axis = WASDNodeController::XAxis;
        direction = WASDNodeController::Negative;
        break;
    case SDL_SCANCODE_A:
        axis = WASDNodeController::YAxis;
        direction = WASDNodeController::Negative;
        break;
    case SDL_SCANCODE_D:
        axis = WASDNodeController::YAxis;
        direction = WASDNodeController::Positive;
        break;
    case SDL_SCANCODE_Q:
        axis = WASDNodeController::ZAxis;
        direction = WASDNodeController::Positive;
        break;
    case SDL_SCANCODE_E:
        axis = WASDNodeController::ZAxis;
        direction = WASDNodeController::Negative;
        break;
    case SDL_SCANCODE_ESCAPE:
        shouldQuit = true;
        break;
    default:
        break;
    }

    if(moving.has_value() && axis.has_value() && direction.has_value())
    {
        wasdController.SetMoving(*axis, *direction, *moving);
    }
}

void SampleApplication::HandleMouseMotionEvent(const SDL_Event& event)
{
    if(event.motion.state & SDL_BUTTON_LMASK)
    {
        if(std::abs(event.motion.xrel) > 5000 || std::abs(event.motion.yrel) > 5000)
        {
            return;
        }

        mouseController.AddMovement(event.motion.xrel, event.motion.yrel);
    }
}
