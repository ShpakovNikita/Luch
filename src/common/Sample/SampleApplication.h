#pragma once

#include <Luch/BaseApplication.h>
#include <Luch/glTF2/glTFParser.h>
#include <Luch/glTF2/glTF.h>

#include <Luch/Render/SceneRenderer.h>

#ifdef _WIN32
    #if LUCH_USE_VULKAN
        #include "../win32/VulkanAllocator.h"
    #endif
    #include <Luch/Platform/Win32/WndProcDelegate.h>
#endif

class SampleApplication
    : public Luch::BaseApplication
#if USE_VULKAN
    , private Luch::Vulkan::VulkanDebugDelegate
#endif
#ifdef _WIN32
    , private Luch::Platform::Win32::WndProcDelegate
#endif
{
public:
    SampleApplication() = default;

    bool Initialize(const Luch::Vector<Luch::String>& args) override;
    bool Deinitialize() override;
    void Run() override;
    void Process();

    const Luch::String& GetApplicationName() const override
    {
        static Luch::String applicationName = "Sample";
        return applicationName;
    }

    const Luch::Version& GetApplicationVersion() const override
    {
        static Luch::Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }

    const Luch::String& GetMainWindowTitle() const override
    {
        static Luch::String windowTitle = "Sample";
        return windowTitle;
    }

    void SetViewSize(Luch::int32 aWidth, Luch::int32 aHeight)
    {
        width = aWidth;
        height = aHeight;
    }
#if __APPLE__
    void SetView(void* aView)
    {
        view = aView;
    }
#endif
private:
#ifdef _WIN32
    LRESULT WndProc(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    ) override;

    std::tuple<HINSTANCE, HWND> CreateMainWindow(const Luch::String& title, Luch::int32 width, Luch::int32 height);
#endif

#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

#if __APPLE__
    void* view = nullptr;
#endif

    Luch::int32 width = 1200;
    Luch::int32 height = 900;

    Luch::RefPtr<Luch::Graphics::PhysicalDevice> physicalDevice;
    Luch::RefPtr<Luch::Graphics::Surface> surface;
#if WIN32
    VulkanAllocator allocator;
#endif

    Luch::SharedPtr<Luch::Render::RenderContext> context;
    Luch::UniquePtr<Luch::Render::SceneRenderer> renderer;
    Luch::RefPtr<Luch::SceneV1::Scene> scene;
    Luch::RefPtr<Luch::SceneV1::Node> cameraNode;
};
