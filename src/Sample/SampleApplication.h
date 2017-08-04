#pragma once

#include <Husky/Application.h>

class SampleApplication : public Husky::BaseApplication
{
public:
    void Initialize(const Vector<String>& args) override;
    void Deinitialize() override;
    void Run() override;
protected:
    const String& GetApplicationName()
    {
        static String applicationName = "Sample";
        return applicationName;
    }

    const Version& GetApplicationVersion()
    {
        static Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }
private:
    Vector<String> GetRequiredExtensionNames();
    Vector<String> GetValidationLayerNames();

    VulkanInstance instance;
    VkUniquePtr<VkDebugReportCallbackEXT> debugCallback;

    VkBool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char * pLayerPrefix,
        const char * pMessage) override;
};