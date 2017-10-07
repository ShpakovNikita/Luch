#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Buffer
    {
        friend class GraphicsDevice;
    public:
        Buffer() = default;

        Buffer(Buffer&& other);
        Buffer& operator=(Buffer&& other);

        ~Buffer();

        VulkanResultValue<void*> MapMemory(int64 size, int64 offset);
        void UnmapMemory();

        // TODO more control
        vk::Result FlushMemory();
        vk::Result InvalidateMemory();

        inline vk::Buffer GetBuffer() { return buffer; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
    private:
        Buffer(GraphicsDevice* device, vk::Buffer buffer, vk::DeviceMemory memory, vk::BufferCreateInfo bufferCreateInfo);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::BufferCreateInfo bufferCreateInfo;
    };
}
