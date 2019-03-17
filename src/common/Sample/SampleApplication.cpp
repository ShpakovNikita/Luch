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
#endif
#if LUCH_USE_VULKAN
    #include <vulkan/vulkan.hpp>
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_vulkan.h>
    #include <Luch/Vulkan/VulkanInstance.h>
    #include <Luch/Vulkan/VulkanPhysicalDevice.h>
    #include <Luch/Vulkan/VulkanSurface.h>
#endif

#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Render/SceneRenderer.h>
#include <Luch/Render/SceneRendererConfig.h>
#include <Luch/Render/RenderUtils.h>
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

bool SampleApplication::Initialize(const Vector<String>& /*args*/)
{
    if (!CreateWindow())
    {
        return false;
    }

    if (!SetupScene())
    {
        return false;
    }

    context = MakeShared<Render::RenderContext>();

    auto [createDeviceResult, createdDevice] = physicalDevice->CreateGraphicsDevice();
    if (createDeviceResult != GraphicsResult::Success)
    {
        return false;
    }

    context->device = std::move(createdDevice);

    auto [createCommandQueueResult, createdCommandQueue] = context->device->CreateCommandQueue();
    if (createCommandQueueResult != GraphicsResult::Success)
    {
        return false;
    }

    context->commandQueue = std::move(createdCommandQueue);

    SwapchainInfo swapchainInfo;
    swapchainInfo.imageCount = 1;
    swapchainInfo.width = width;
    swapchainInfo.height = height;

    auto [createSwapchainResult, createdSwapchain] = context->device->CreateSwapchain(swapchainInfo, surface);
    if (createSwapchainResult != GraphicsResult::Success)
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

bool SampleApplication::CreateWindow()
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

#if LUCH_USE_VULKAN
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window* window = SDL_CreateWindow(
        "Sample app",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        500, 500,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    Luch::Vulkan::VulkanInstance vulkanInstance;
    if (!vulkanInstance.Init())
    {
        LUCH_ASSERT_MSG(false, "Failed to create Vulkan Instance");
        return false;
    }

    vk::Instance vkInstance = vulkanInstance.GetInstance();
    VkSurfaceKHR vkSurface = nullptr;
    SDL_bool result = SDL_Vulkan_CreateSurface(window, vkInstance, &vkSurface);

    if (window == 0 || result != SDL_TRUE)
    {
        LUCH_ASSERT_MSG(false, "Failed to create Vulkan Surface");
        return false;
    }
    surface = MakeRef<Luch::Vulkan::VulkanSurface>(vkInstance, vkSurface, nullptr);

    // if you need allocation callbacks store it somewhere
    physicalDevice = MakeRef<Luch::Vulkan::VulkanPhysicalDevice>(vkInstance, vkSurface, nullptr);
    if (!physicalDevice->Init())
    {
        LUCH_ASSERT_MSG(false, "Failed to create Vulkan Physical Device");
        return false;
    }

#endif

    LUCH_ASSERT_MSG(physicalDevice.Get() != nullptr, "Uninitialized physical device");
    return true;
}

bool SampleApplication::SetupScene()
{
    glTF::glTFParser glTFparser;

    String rootDir{ "../res/gltf2/helmet/" };

    {
        String filename { "DamagedHelmet.gltf" };

        FileStream fileStream{ rootDir + filename, FileOpenModes::Read };
        
        auto root = glTFparser.ParseJSON(&fileStream);

        SceneV1::Loader::glTFLoader loader{ rootDir, root };
        scene = loader.LoadScene(0);
    }

    auto cameraIt = std::find_if(
        scene->GetNodes().begin(),
        scene->GetNodes().end(),
        [](const auto& node) { return node->GetCamera() != nullptr; });

    auto directionalLightIt = std::find_if(
        scene->GetNodes().begin(),
        scene->GetNodes().end(),
        [](const auto& node) { return node->GetLight() != nullptr && node->GetLight()->GetType() == SceneV1::LightType::Directional; });

    LUCH_ASSERT(cameraIt != scene->GetNodes().end());
    cameraNode = *cameraIt;

    if(directionalLightIt != scene->GetNodes().end())
    {
        directionalLightNode = *directionalLightIt;
    }

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
    swapchainInfo.format = Format::BGRA8Unorm_sRGB;
    swapchainInfo.imageCount = 3;
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

    renderer->GetMutableConfig().useGlobalIllumination = true;
    renderer->GetMutableConfig().useDiffuseGlobalIllumination = true;
    renderer->GetMutableConfig().useSpecularGlobalIllumination = true;
    renderer->GetMutableConfig().useForward = true;
    renderer->GetMutableConfig().useDepthPrepass = true;
    renderer->GetMutableConfig().useComputeResolve = false;
    renderer->GetMutableConfig().useTiledDeferredPass = false;

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
#if LUCH_USE_VULKAN
    SDL_Quit();
#endif

    return true;
}

bool SampleApplication::ShouldQuit() const
{
    return shouldQuit;
}

void SampleApplication::Process()
{
    mouseController.Tick();
    wasdController.Tick(16.0f / 1000.0f);
    scene->Update();

    bool resourcesPrepared = renderer->PrepareSceneResources();
    if(!resourcesPrepared)
    {
        return;
    }

    if((probeIndirectEveryFrame || !indirectProbed))
    {
        indirectProbed = renderer->ProbeIndirectLighting();
        LUCH_ASSERT(indirectProbed);
    }

    bool beginSucceeded = renderer->BeginRender();
    if(!beginSucceeded)
    {
        return;
    }

    bool prepareSucceeded = renderer->PrepareScene();
    if(!prepareSucceeded)
    {
        LUCH_ASSERT(false);
        return;
    }

    renderer->UpdateScene();
    renderer->DrawScene(cameraNode);
    renderer->EndRender();
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
    switch(event.key.keysym.scancode)
    {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_S:
    case SDL_SCANCODE_A:
    case SDL_SCANCODE_D:
    case SDL_SCANCODE_Q:
    case SDL_SCANCODE_E:
        HandleKeyboardMovementEvent(event);
        break;
    default:
        break;
    }

    if(event.type == SDL_KEYUP)
    {
        switch(event.key.keysym.scancode)
        {
        case SDL_SCANCODE_ESCAPE:
            shouldQuit = true;
            break;
        case SDL_SCANCODE_Z:
            renderer->GetMutableConfig().useDepthPrepass = true;
            break;
        case SDL_SCANCODE_X:
            renderer->GetMutableConfig().useDepthPrepass = false;
            break;
        case SDL_SCANCODE_1:
            renderer->GetMutableConfig().useForward = true;
            renderer->GetMutableConfig().useComputeResolve = false;
            renderer->GetMutableConfig().useTiledDeferredPass = false;
            break;
        case SDL_SCANCODE_2:
            renderer->GetMutableConfig().useForward = false;
            renderer->GetMutableConfig().useComputeResolve = false;
            renderer->GetMutableConfig().useTiledDeferredPass = false;
            break;
        case SDL_SCANCODE_3:
            renderer->GetMutableConfig().useForward = false;
            renderer->GetMutableConfig().useComputeResolve = true;
            renderer->GetMutableConfig().useTiledDeferredPass = false;
            break;
        case SDL_SCANCODE_C:
            directionalLightNode->GetLight()->SetEnabled(!directionalLightNode->GetLight()->IsEnabled());
            break;
        case SDL_SCANCODE_V:
            renderer->GetMutableConfig().useDiffuseGlobalIllumination = !renderer->GetMutableConfig().useDiffuseGlobalIllumination;
            break;
        case SDL_SCANCODE_B:
            renderer->GetMutableConfig().useSpecularGlobalIllumination = !renderer->GetMutableConfig().useSpecularGlobalIllumination;
            break;
        case SDL_SCANCODE_I:
            indirectProbed = false;
            break;
        default:
            break;
        }
    }
}

void SampleApplication::HandleKeyboardMovementEvent(const SDL_Event& event)
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
