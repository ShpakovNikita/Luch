#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>

#include <Husky/Render/DeferredRenderer.h>

#ifdef _WIN32
    #if HUSKY_USE_VULKAN
        #include "../win32/VulkanAllocator.h"
    #endif
    #include <Husky/Platform/Win32/WndProcDelegate.h>
#endif

class SampleApplication
    : public Husky::BaseApplication
#if USE_VULKAN
    , private Husky::Vulkan::VulkanDebugDelegate
#endif
#ifdef _WIN32
    , private Husky::Platform::Win32::WndProcDelegate
#endif
{
public:
    SampleApplication() = default;

    bool Initialize(const Husky::Vector<Husky::String>& args) override;
    bool Deinitialize() override;
    void Run() override;
    void Process();

    const Husky::String& GetApplicationName() const override
    {
        static Husky::String applicationName = "Sample";
        return applicationName;
    }

    const Husky::Version& GetApplicationVersion() const override
    {
        static Husky::Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }

    const Husky::String& GetMainWindowTitle() const override
    {
        static Husky::String windowTitle = "Sample";
        return windowTitle;
    }

    void SetViewSize(Husky::int32 aWidth, Husky::int32 aHeight)
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

    std::tuple<HINSTANCE, HWND> CreateMainWindow(const Husky::String& title, Husky::int32 width, Husky::int32 height);
#endif

#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

#if __APPLE__
    void* view = nullptr;
#endif

    Husky::int32 width = 1200;
    Husky::int32 height = 900;

    Husky::RefPtr<Husky::Graphics::PhysicalDevice> physicalDevice;
    Husky::RefPtr<Husky::Graphics::Surface> surface;
#if WIN32
    VulkanAllocator allocator;
#endif
    Husky::Render::DeferredPreparedScene preparedScene;

    Husky::UniquePtr<Husky::Render::DeferredRenderer> deferredRenderer;
};
