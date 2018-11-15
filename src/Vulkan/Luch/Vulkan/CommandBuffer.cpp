#include <Luch/Vulkan/VulkanCommandBuffer.h>

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
