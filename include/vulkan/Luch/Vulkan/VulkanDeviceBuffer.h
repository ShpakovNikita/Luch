#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanDeviceBuffer : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDeviceBuffer(
            VulkanGraphicsDevice* device,
            vk::Buffer buffer,
            vk::DeviceMemory memory,
            vk::BufferCreateInfo bufferCreateInfo);

        ~VulkanDeviceBuffer() override;

        GraphicsResultValue<void*> MapMemory(int64 size, int64 offset);
        void* GetMappedMemory() const { return mappedMemory; }
        void UnmapMemory();

        // TODO more control
        vk::Result FlushMemory();
        vk::Result InvalidateMemory();

        inline VulkanGraphicsDevice* GetDevice() { return device; }
        inline vk::Buffer GetBuffer() { return buffer; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::BufferCreateInfo bufferCreateInfo;

        void* mappedMemory = nullptr;
    };
}
