#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/GlslShaderCompiler.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>
#include <Husky/Vulkan/Swapchain.h>

#include <Husky/Render/ForwardRenderer.h>
#include <Husky/Render/DeferredRenderer.h>

#include "VulkanAllocator.h"

#ifdef _WIN32
#include <Husky/Platform/Win32/WndProcDelegate.h>
#endif

class SampleApplication
    : public Husky::BaseApplication
    , private Husky::Vulkan::VulkanDebugDelegate
#ifdef _WIN32
    , private Husky::Platform::Win32::WndProcDelegate
#endif
{
public:
    SampleApplication() = default;
    
    bool Initialize(const Husky::Vector<Husky::String>& args) override;
    bool Deinitialize() override;
    void Run() override;

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
private:
    vk::ResultValue<vk::Instance> CreateVulkanInstance();

    vk::ResultValue<vk::DebugReportCallbackEXT> CreateDebugCallback(vk::Instance& instance);

    void DestroyDebugCallback(
        vk::Instance& instance,
        vk::DebugReportCallbackEXT& callback);

    vk::PhysicalDevice ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices);

#ifdef _WIN32
    LRESULT WndProc(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    ) override;

    std::tuple<HINSTANCE, HWND> CreateMainWindow(const Husky::String& title, Husky::int32 width, Husky::int32 height);
#endif

    Husky::Vector<const Husky::char8*> GetRequiredInstanceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetValidationLayerNames() const;

    vk::Bool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char * pLayerPrefix,
        const char * pMessage) override;

#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

    Husky::int32 width = 1200;
    Husky::int32 height = 900;

    Husky::Vulkan::PhysicalDevice physicalDevice;
    Husky::Vulkan::Surface surface;
    VulkanAllocator allocator;
    vk::Instance instance;
    vk::DebugReportCallbackEXT debugCallback;
    Husky::Render::DeferredPreparedScene preparedScene;

    Husky::Optional<vk::AllocationCallbacks> allocationCallbacks;

    Husky::UniquePtr<Husky::Render::ForwardRenderer> forwardRenderer;
    Husky::UniquePtr<Husky::Render::DeferredRenderer> deferredRenderer;
};
