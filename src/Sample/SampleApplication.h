#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/Vulkan.h>

class SampleApplication 
    : public Husky::BaseApplication
    , private Husky::VulkanDebugDelegate
{
public:
    SampleApplication() = default;
    void Initialize(const Husky::Vector<Husky::String>& args) override;
    void Deinitialize() override;
    void Run() override;
protected:
    const Husky::String& GetApplicationName()
    {
        static Husky::String applicationName = "Sample";
        return applicationName;
    }

    const Husky::Version& GetApplicationVersion()
    {
        static Husky::Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }
private:
    struct QueueInfo
    {
        vk::Queue queue;
        uint32 queueFamilyIndex;
    };

    vk::ResultValue<vk::Instance> CreateVulkanInstance(vk::AllocationCallbacks& allocationCallbacks);
    vk::ResultValue<vk::DebugReportCallbackEXT> CreateDebugCallback(vk::Instance& instance, vk::AllocationCallbacks& allocationCallbacks);
    vk::PhysicalDevice ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices);
    Husky::Optional<vk::DeviceQueueCreateInfo> ChooseDeviceQueue(vk::PhysicalDevice& physicalDevice);

    vk::ResultValue<vk::Device> CreateDevice(
        vk::PhysicalDevice& physicalDevice,
        vk::DeviceQueueCreateInfo& queueCreateInfo,
        vk::AllocationCallbacks& allocationCallbacks);

    vk::ResultValue<vk::CommandPool> CreateCommandPool(
        vk::Device& device,
        const QueueInfo& info,
        vk::AllocationCallbacks& allocationCallbacks);

    Husky::Vector<const Husky::char8*> GetRequiredInstanceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetRequiredDeviceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetValidationLayerNames() const;

    vk::AllocationCallbacks allocationCallbacks;
    vk::Instance instance;
    vk::DebugReportCallbackEXT debugCallback;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    QueueInfo queueInfo;
    vk::CommandPool commandPool;

    vk::Bool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char * pLayerPrefix,
        const char * pMessage) override;
};