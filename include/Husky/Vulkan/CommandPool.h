#pragma once

#include <Husky/Vulkan.h>
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

    class CommandPool
    {
        friend class GraphicsDevice;
    public:
        CommandPool() = default;

        ~CommandPool();

        CommandPool(CommandPool&& other);
        CommandPool& operator=(CommandPool&& other);

        inline vk::CommandPool GetCommandPool() { return commandPool; }

        VulkanResultValue<Vector<CommandBuffer>> AllocateCommandBuffers(int32 count, CommandBufferLevel level);
        vk::Result Reset(bool releaseResources);
    private:
        CommandPool(GraphicsDevice* device, vk::CommandPool commandPool);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::CommandPool commandPool;
    };
}
