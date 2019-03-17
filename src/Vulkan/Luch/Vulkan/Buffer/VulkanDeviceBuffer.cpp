#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Buffer/VulkanDeviceBuffer.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanDeviceBuffer::VulkanDeviceBuffer(
        VulkanGraphicsDevice* aDevice,
        vk::Buffer aBuffer,
        vk::DeviceMemory aMemory,
        vk::BufferCreateInfo aBufferCreateInfo)
        : Buffer(aDevice)
        , device(aDevice)
        , buffer(aBuffer)
        , memory(aMemory)
        , bufferCreateInfo(aBufferCreateInfo)
    {
    }

    VulkanDeviceBuffer::~VulkanDeviceBuffer()
    {
        Destroy();
    }

    GraphicsResultValue<void*> VulkanDeviceBuffer::MapMemory(
        int32 size,
        int32 offset)
    {
        if (size == 0)
        {
            return GraphicsResult::Success; // todo: is it valid call? not for vulkan
        }
        auto [result, memoryPointer] = device->GetDevice().mapMemory(memory, offset, size);
        mappedMemory = memoryPointer;
        return { result, memoryPointer };
    }

    GraphicsResult VulkanDeviceBuffer::UnmapMemory()
    {
        mappedMemory = nullptr;
        device->GetDevice().unmapMemory(memory);
        return GraphicsResult::Success;
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
