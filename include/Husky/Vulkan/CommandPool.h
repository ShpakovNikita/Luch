#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>
#include <Husky/Vulkan/CommandBuffer.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class CommandBuffer;

    enum class CommandBufferLevel
    {
        Primary,
        Secondary,
    };

    class CommandPool : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        CommandPool(GraphicsDevice* device, vk::CommandPool commandPool);

        CommandPool(const CommandPool& other) = delete;
        CommandPool(CommandPool&& other) = delete;
        CommandPool& operator=(const CommandPool& other) = delete;
        CommandPool& operator=(CommandPool&& other) = delete;

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
