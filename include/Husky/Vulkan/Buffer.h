#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class Buffer : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Buffer(
            GraphicsDevice* device,
            vk::Buffer buffer,
            vk::DeviceMemory memory,
            vk::BufferCreateInfo bufferCreateInfo);

        ~Buffer() override;

        VulkanResultValue<void*> MapMemory(int64 size, int64 offset);
        void* GetMappedMemory() const { return mappedMemory; }
        void UnmapMemory();

        // TODO more control
        vk::Result FlushMemory();
        vk::Result InvalidateMemory();

        inline GraphicsDevice* GetDevice() { return device; }
        inline vk::Buffer GetBuffer() { return buffer; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::BufferCreateInfo bufferCreateInfo;

        void* mappedMemory = nullptr;
    };
}
