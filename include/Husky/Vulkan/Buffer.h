#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class Device;

    class Buffer
    {
        friend class Device;
    public:
        Buffer();

        Buffer(Buffer&& other);
        Buffer& operator=(Buffer&& other);

        inline vk::Buffer GetBuffer() { return buffer; }
    private:
        Buffer(Device* device, vk::Buffer buffer, vk::BufferCreateInfo bufferCreateInfo);
        Device* device = nullptr;
        vk::Buffer buffer;
        vk::BufferCreateInfo bufferCreateInfo;
    };
}
