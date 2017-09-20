#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/Device.h>

namespace Husky::Vulkan
{
    Buffer::Buffer(Buffer&& other)
        : device(other.device)
        , buffer(other.buffer)
    {
        other.device = nullptr;
        other.buffer = nullptr;
    }

    Buffer& Buffer::operator=(Buffer&& other)
    {
        device = other.device;
        buffer = other.buffer;
        other.device = nullptr;
        other.buffer = nullptr;
        return *this;
    }

    Buffer::Buffer(Device* aDevice, vk::Buffer aBuffer, vk::BufferCreateInfo aBufferCreateInfo)
        : device(aDevice)
        , buffer(aBuffer)
        , bufferCreateInfo(aBufferCreateInfo)
    {

    }
}