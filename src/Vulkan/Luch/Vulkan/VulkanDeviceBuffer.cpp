#include <Luch/Vulkan/VulkanDeviceBuffer.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanDeviceBuffer::VulkanDeviceBuffer(
        VulkanGraphicsDevice* aDevice,
        vk::Buffer aBuffer,
        vk::DeviceMemory aMemory,
        vk::BufferCreateInfo aBufferCreateInfo)
        : device(aDevice)
        , buffer(aBuffer)
        , memory(aMemory)
        , bufferCreateInfo(aBufferCreateInfo)
    {
    }

    VulkanDeviceBuffer::~VulkanDeviceBuffer()
    {
        Destroy();
    }

    VulkanResultValue<void*> VulkanDeviceBuffer::MapMemory(
        int64 size,
        int64 offset)
    {
        auto [result, memoryPointer] = device->GetDevice().mapMemory(memory, offset, size);
        mappedMemory = memoryPointer;
        return { result, memoryPointer };
    }

    void VulkanDeviceBuffer::UnmapMemory()
    {
        mappedMemory = nullptr;
        device->GetDevice().unmapMemory(memory);
    }

    vk::Result VulkanDeviceBuffer::FlushMemory()
    {
        vk::MappedMemoryRange memoryRange;
        memoryRange.setMemory(memory);
        memoryRange.setSize(VK_WHOLE_SIZE);
        return device->GetDevice().flushMappedMemoryRanges({ memoryRange });
    }

    vk::Result VulkanDeviceBuffer::InvalidateMemory()
    {
        vk::MappedMemoryRange memoryRange;
        memoryRange.setMemory(memory);
        memoryRange.setSize(VK_WHOLE_SIZE);
        return device->GetDevice().invalidateMappedMemoryRanges({ memoryRange });
    }

    void VulkanDeviceBuffer::Destroy()
    {
        if (device)
        {
            device->DestroyBuffer(this);
        }
    }
}
