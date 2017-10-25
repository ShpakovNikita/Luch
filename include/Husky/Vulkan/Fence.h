#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Fence
    {
        friend class GraphicsDevice;
    public:
        Fence() = default;

        Fence(Fence&& other);
        Fence& operator=(Fence&& other);

        ~Fence();

        inline vk::Fence GetFence() { return fence; }
    private:
        Fence(GraphicsDevice* device, vk::Fence fence);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Fence fence;
    };
}
