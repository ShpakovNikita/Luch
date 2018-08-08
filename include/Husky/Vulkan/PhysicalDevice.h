#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/QueueInfo.h>
#include <Husky/Format.h>

namespace Husky::Vulkan
{
    class Surface;

    class PhysicalDevice : public BaseObject
    {
    public:
        PhysicalDevice() = default;

        PhysicalDevice(
            vk::PhysicalDevice physicalDevice,
            Husky::Optional<vk::AllocationCallbacks> allocationCallbacks);

        PhysicalDevice(PhysicalDevice&& other) = default;
        PhysicalDevice& operator=(PhysicalDevice&& other) = default;

        inline vk::PhysicalDevice GetPhysicalDevice() { return physicalDevice; }
        inline const vk::Optional<const vk::AllocationCallbacks>& GetAllocationCAllbacks() const { return allocationCallbacks; }
        inline const vk::PhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return physicalDeviceMemoryProperties; }

        VulkanResultValue<QueueIndices> ChooseDeviceQueues(Surface* surface);

        VulkanRefResultValue<GraphicsDevice> CreateDevice(
            QueueIndices&& queueIndices,
            const Husky::Vector<const char8*>& requiredDeviceExtensionNames);

        Vector<Format> GetSupportedDepthStencilFormats(const Vector<Format>& formats);
    private:
        QueueInfo ObtainQueueInfo(vk::Device & device, QueueIndices&& indices);

        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

        // Workaround for vk::Optional
        Husky::Optional<vk::AllocationCallbacks> callbacks;
        vk::Optional<const vk::AllocationCallbacks> allocationCallbacks = nullptr;
    };
}
