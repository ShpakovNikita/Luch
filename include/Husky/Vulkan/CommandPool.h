#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class CommandPool
    {
        friend class GraphicsDevice;
    public:
        CommandPool() = default;

        ~CommandPool();

        CommandPool(CommandPool&& other);
        CommandPool& operator=(CommandPool&& other);

        inline vk::CommandPool GetCommandPool() { return commandPool; }

        vk::Result Reset(bool releaseResources);
    private:
        CommandPool(GraphicsDevice* device, vk::CommandPool commandPool);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::CommandPool commandPool;
    };
}
