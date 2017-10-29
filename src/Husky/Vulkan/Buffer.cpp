#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Buffer::Buffer(
        GraphicsDevice* aDevice,
        vk::Buffer aBuffer,
        vk::DeviceMemory aMemory,
        vk::BufferCreateInfo aBufferCreateInfo)
        : device(aDevice)
        , buffer(aBuffer)
        , memory(aMemory)
        , bufferCreateInfo(aBufferCreateInfo)
    {
    }

    Buffer::Buffer(Buffer&& other)
        : device(other.device)
        , buffer(other.buffer)
        , memory(other.memory)
        , bufferCreateInfo(other.bufferCreateInfo)
    {
        other.device = nullptr;
        other.buffer = nullptr;
        other.memory = nullptr;
    }

    Buffer& Buffer::operator=(Buffer&& other)
    {
        Destroy();

        device = other.device;
        buffer = other.buffer;
        memory = other.memory;
        bufferCreateInfo = other.bufferCreateInfo;

        other.device = nullptr;
        other.buffer = nullptr;
        other.memory = nullptr;

        return *this;
    }

    Buffer::~Buffer()
    {
        Destroy();
    }

    VulkanResultValue<void*> Buffer::MapMemory(int64 size, int64 offset)
    {
        auto [result, memoryPointer] = device->GetDevice().mapMemory(memory, offset, size);
        return { result, memoryPointer };
    }

    void Buffer::UnmapMemory()
    {
        device->GetDevice().unmapMemory(memory);
    }

    // TODO more control

    vk::Result Buffer::FlushMemory()
    {
        vk::MappedMemoryRange memoryRange;
        memoryRange.setMemory(memory);
        memoryRange.setSize(VK_WHOLE_SIZE);
        return device->GetDevice().flushMappedMemoryRanges({ memoryRange });
    }

    vk::Result Buffer::InvalidateMemory()
    {
        vk::MappedMemoryRange memoryRange;
        memoryRange.setMemory(memory);
        memoryRange.setSize(VK_WHOLE_SIZE);
        return device->GetDevice().invalidateMappedMemoryRanges({ memoryRange });
    }

    void Buffer::Destroy()
    {
        if (device)
        {
            device->DestroyBuffer(this);
        }
    }
}
