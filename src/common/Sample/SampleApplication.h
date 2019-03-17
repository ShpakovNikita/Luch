#pragma once

#include <SDL_events.h>

#include <Luch/BaseApplication.h>
#include <Luch/glTF2/glTFParser.h>
#include <Luch/glTF2/glTF.h>
#include <Luch/WASDNodeController.h>
#include <Luch/MouseNodeController.h>
#include <Luch/Render/SceneRenderer.h>

class SampleApplication : public Luch::BaseApplication
{
public:
    SampleApplication() = default;

    bool Initialize(const Luch::Vector<Luch::String>& args) override;
    bool Deinitialize() override;
    bool ShouldQuit() const override;
    void Process();

    void HandleEvent(const SDL_Event& event);

    const Luch::String& GetApplicationName() const override
    {
        static Luch::String applicationName = "Sample";
        return applicationName;
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
    bool CreateWindow();
    void SetupScene();
    void HandleKeyboardEvent(const SDL_Event& event);
    void HandleKeyboardMovementEvent(const SDL_Event& event);
    void HandleMouseMotionEvent(const SDL_Event& event);

#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

#if __APPLE__
    void* view = nullptr;
#endif

    Luch::int32 width = 1000;
    Luch::int32 height = 1000;
    bool shouldQuit = false;

    Luch::RefPtr<Luch::Graphics::PhysicalDevice> physicalDevice;
    Luch::RefPtr<Luch::Graphics::Surface> surface;

    Luch::SharedPtr<Luch::Render::RenderContext> context;
    Luch::UniquePtr<Luch::Render::SceneRenderer> renderer;
    Luch::RefPtr<Luch::SceneV1::Scene> scene;
    Luch::RefPtr<Luch::SceneV1::Node> cameraNode;
    Luch::RefPtr<Luch::SceneV1::Node> directionalLightNode;
    Luch::WASDNodeController wasdController;
    Luch::MouseNodeController mouseController;

    bool indirectProbed = false;
    bool probeIndirectEveryFrame = false;
};
