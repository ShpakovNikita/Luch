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
        CommandPool(GraphicsDevice* device, vk::CommandPool commandPool);
        ~CommandPool() override;

        inline vk::CommandPool GetCommandPool() { return commandPool; }

        VulkanResultValue<RefPtrVector<CommandBuffer>> AllocateCommandBuffers(int32 count, CommandBufferLevel level);
        VulkanRefResultValue<CommandBuffer> AllocateCommandBuffer(CommandBufferLevel level);

        vk::Result Reset(bool releaseResources = false);
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::CommandPool commandPool;
    };
}
