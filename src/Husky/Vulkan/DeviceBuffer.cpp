#include <Husky/Vulkan/DeviceBuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    DeviceBuffer::DeviceBuffer(
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

    DeviceBuffer::~DeviceBuffer()
    {
        Destroy();
    }

    VulkanResultValue<void*> DeviceBuffer::MapMemory(int64 size, int64 offset)
    {
        auto [result, memoryPointer] = device->GetDevice().mapMemory(memory, offset, size);
        mappedMemory = memoryPointer;
        return { result, memoryPointer };
    }

    void DeviceBuffer::UnmapMemory()
    {
        mappedMemory = nullptr;
        device->GetDevice().unmapMemory(memory);
    }

    // TODO more control

    vk::Result DeviceBuffer::FlushMemory()
    {
        vk::MappedMemoryRange memoryRange;
        memoryRange.setMemory(memory);
        memoryRange.setSize(VK_WHOLE_SIZE);
        return device->GetDevice().flushMappedMemoryRanges({ memoryRange });
    }

    vk::Result DeviceBuffer::InvalidateMemory()
    {
        vk::MappedMemoryRange memoryRange;
        memoryRange.setMemory(memory);
        memoryRange.setSize(VK_WHOLE_SIZE);
        return device->GetDevice().invalidateMappedMemoryRanges({ memoryRange });
    }

    void DeviceBuffer::Destroy()
    {
        if (device)
        {
            device->DestroyBuffer(this);
        }
    }
}
