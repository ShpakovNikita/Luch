#include <Luch/Vulkan/VulkanGraphicsCommandList.h>

namespace Luch::Vulkan
{
    VulkanGraphicsCommandList::VulkanGraphicsCommandList(
        VulkanGraphicsDevice* aDevice,
        vk::CommandBuffer aCommandBuffer)
        : GraphicsCommandList(aDevice)
        , device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }

    void VulkanGraphicsCommandList::Begin()
    {
        vk::CommandBufferBeginInfo beginInfo;
        commandBuffer.begin(beginInfo);
    }

    void VulkanGraphicsCommandList::End()
    {
        commandBuffer.end();
    }

}
