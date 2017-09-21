#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class CommandBuffer
    {
        friend class GraphicsDevice;
    public:
        CommandBuffer() = default;
        ~CommandBuffer();

        CommandBuffer(CommandBuffer&& other);
        CommandBuffer& operator=(CommandBuffer&& other);

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }
    private:
        CommandBuffer(GraphicsDevice* device, vk::CommandBuffer commandBuffer);

        GraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}