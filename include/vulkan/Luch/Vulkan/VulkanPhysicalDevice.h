#pragma once

#include <Luch/Vulkan.h>
#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Vulkan/VulkanQueueInfo.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/GraphicsResultValue.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanPhysicalDevice : public Luch::Graphics::PhysicalDevice
    {
    public:
        VulkanPhysicalDevice(vk::Instance aInstance, vk::SurfaceKHR aSurface, Luch::Optional<vk::AllocationCallbacks> allocationCallbacks);

        bool Init();

        inline vk::PhysicalDevice GetPhysicalDevice() { return physicalDevice; }
        inline const vk::Optional<const vk::AllocationCallbacks>& GetAllocationCallbacks() const { return allocationCallbacks; }
        inline const vk::PhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return physicalDeviceMemoryProperties; }

        GraphicsResultValue<QueueIndices> ChooseDeviceQueues(vk::SurfaceKHR surface);

        GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() override;
        Vector<Graphics::Format> GetSupportedDepthStencilFormats(const Vector<Graphics::Format>& formats) const override;

    private:
        VulkanQueueInfo ObtainQueueInfo(vk::Device & device, QueueIndices&& indices);

        vk::Instance instance;
        vk::SurfaceKHR surface;

        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

        // Workaround for vk::Optional
        Luch::Optional<vk::AllocationCallbacks> callbacks;
        vk::Optional<const vk::AllocationCallbacks> allocationCallbacks = nullptr;
    };
}
