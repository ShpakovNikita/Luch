#include <Husky/Vulkan/CommandBuffer.h>

namespace Husky::Vulkan
{
    CommandBuffer::CommandBuffer(GraphicsDevice * aDevice, vk::CommandBuffer aCommandBuffer)
        : device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }

    CommandBuffer::CommandBuffer(CommandBuffer && other)
        : device(other.device)
        , commandBuffer(other.commandBuffer)
    {
        other.device = nullptr;
        other.commandBuffer = nullptr;
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other)
    {
        device = other.device;
        commandBuffer = other.commandBuffer;

        other.device = nullptr;
        other.commandBuffer = nullptr;

        return *this;
    }
}
