#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class CommandPool
    {
    public:
        CommandPool() = default;

    private:
        vk::CommandPool commandPool;
    };
}