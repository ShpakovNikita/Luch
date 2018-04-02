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

    Buffer::~Buffer()
    {
        Destroy();
    }

    VulkanResultValue<void*> Buffer::MapMemory(int64 size, int64 offset)
    {
        auto [result, memoryPointer] = device->GetDevice().mapMemory(memory, offset, size);
        mappedMemory = memoryPointer;
        return { result, memoryPointer };
    }

    void Buffer::UnmapMemory()
    {
        mappedMemory = nullptr;
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
