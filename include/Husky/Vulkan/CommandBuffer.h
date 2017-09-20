#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class Device;

    class CommandBuffer
    {
        friend class Device;
    public:
        CommandBuffer() = default;
        ~CommandBuffer();

        CommandBuffer(CommandBuffer&& other);
        CommandBuffer& operator=(CommandBuffer&& other);

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }
    private:
        CommandBuffer(Device* device, vk::CommandBuffer commandBuffer);

        Device* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}