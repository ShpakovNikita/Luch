#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Device.h>
#include <Husky/Vulkan/QueueInfo.h>

namespace Husky::Vulkan
{
    class Surface;

    class PhysicalDevice
    {
    public:
        PhysicalDevice() = default;

        PhysicalDevice(vk::PhysicalDevice physicalDevice, vk::AllocationCallbacks allocationCallbacks);

        PhysicalDevice(PhysicalDevice&& other) = default;
        PhysicalDevice& operator=(PhysicalDevice&& other) = default;

        inline vk::PhysicalDevice GetPhysicalDevice() { return physicalDevice; }
        inline vk::AllocationCallbacks GetAllocationCAllbacks() const { return allocationCallbacks; }
        inline const vk::PhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return physicalDeviceMemoryProperties; }

        VulkanResultValue<QueueIndices> ChooseDeviceQueues(Surface* surface);

        VulkanResultValue<Device> CreateDevice(
            QueueIndices&& queueIndices,
            const Husky::Vector<const char8*>& requiredDeviceExtensionNames);
    private:
        QueueInfo ObtainQueueInfo(vk::Device & device, QueueIndices&& indices);

        vk::PhysicalDevice physicalDevice;
        vk::AllocationCallbacks allocationCallbacks;
        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    };
}
