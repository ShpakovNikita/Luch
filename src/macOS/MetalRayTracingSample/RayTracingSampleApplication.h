
#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>

class RayTracingSampleApplication
    : public Husky::BaseApplication
{
public:
    RayTracingSampleApplication() = default;

    bool Initialize(const Husky::Vector<Husky::String>& args) override;
    bool Deinitialize() override;
    void Run() override;
    void Process();

    const Husky::String& GetApplicationName() const override
    {
        static Husky::String applicationName = "Ray Tracing Sample";
        return applicationName;
    }

    const Husky::Version& GetApplicationVersion() const override
    {
        static Husky::Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }

    const Husky::String& GetMainWindowTitle() const override
    {
        static Husky::String windowTitle = "Ray Tracing Sample";
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
#if __APPLE__
    void* view = nullptr;
#endif

    Husky::int32 width = 1200;
    Husky::int32 height = 900;
};
