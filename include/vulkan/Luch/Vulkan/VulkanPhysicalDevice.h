#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/QueueInfo.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanPhysicalDevice : public BaseObject
    {
    public:
        VulkanPhysicalDevice(
            vk::PhysicalDevice physicalDevice,
            Luch::Optional<vk::AllocationCallbacks> allocationCallbacks);

        inline vk::PhysicalDevice GetPhysicalDevice() { return physicalDevice; }
        inline const vk::Optional<const vk::AllocationCallbacks>& GetAllocationCAllbacks() const { return allocationCallbacks; }
        inline const vk::PhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return physicalDeviceMemoryProperties; }

        VulkanResultValue<QueueIndices> ChooseDeviceQueues(Surface* surface);

        VulkanRefResultValue<GraphicsDevice> CreateDevice(
            QueueIndices&& queueIndices,
            const Luch::Vector<const char8*>& requiredDeviceExtensionNames);

        Vector<Format> GetSupportedDepthStencilFormats(const Vector<Format>& formats);
    private:
        QueueInfo ObtainQueueInfo(vk::Device & device, QueueIndices&& indices);

        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

        // Workaround for vk::Optional
        Luch::Optional<vk::AllocationCallbacks> callbacks;
        vk::Optional<const vk::AllocationCallbacks> allocationCallbacks = nullptr;
    };
}
