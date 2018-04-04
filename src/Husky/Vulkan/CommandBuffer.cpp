#include <Husky/Vulkan/CommandBuffer.h>

namespace Husky::Vulkan
{
    CommandBuffer::CommandBuffer(GraphicsDevice * aDevice, vk::CommandBuffer aCommandBuffer)
        : device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }
}
