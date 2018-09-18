#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan/VulkanCommandBuffer.h>

namespace Husky::Vulkan
{
    enum class CommandBufferLevel
    {
        Primary,
        Secondary,
    };

    class VulkanCommandPool : public BaseObject
    {
        friend class GraphicsDevice;
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
