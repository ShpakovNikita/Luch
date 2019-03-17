#include <Luch/Vulkan/Queue/VulkanCommandBuffer.h>

namespace Luch::Vulkan
{
    VulkanCommandBuffer::VulkanCommandBuffer(
        VulkanGraphicsDevice* aDevice,
        vk::CommandBuffer aCommandBuffer)
        : device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }
}
