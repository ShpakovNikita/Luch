#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Vulkan/VulkanCommandBuffer.h>

namespace Luch::Vulkan
{
    enum class CommandBufferLevel
    {
        Primary,
        Secondary,
    };

    class VulkanCommandPool : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanCommandPool(
            VulkanGraphicsDevice* device,
            vk::CommandPool commandPool);
        ~VulkanCommandPool() override;

        inline vk::CommandPool GetCommandPool() { return commandPool; }

        VulkanResultValue<RefPtrVector<VulkanCommandBuffer>> AllocateCommandBuffers(
            int32 count,
            CommandBufferLevel level);

        VulkanRefResultValue<VulkanCommandBuffer> AllocateCommandBuffer(CommandBufferLevel level);

        vk::Result Reset(bool releaseResources = false);
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::CommandPool commandPool;
    };
}
