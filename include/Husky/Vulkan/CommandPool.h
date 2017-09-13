#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class Device;

    class CommandPool
    {
        friend class Device;
    public:
        CommandPool() = default;

        ~CommandPool();

        CommandPool(CommandPool&& other);
        CommandPool& operator=(CommandPool&& other);

        inline vk::CommandPool GetCommandPool() { return commandPool; }

        vk::Result Reset(bool releaseResources);
    private:
        CommandPool(Device* device, vk::CommandPool commandPool);

        Device* device = nullptr;
        vk::CommandPool commandPool;
    };
}