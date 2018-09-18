#include <Husky/Vulkan/VulkanCommandBuffer.h>

namespace Husky::Vulkan
{
    VulkanCommandBuffer::VulkanCommandBuffer(
        VulkanGraphicsDevice* aDevice,
        vk::CommandBuffer aCommandBuffer)
        : device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }
}
