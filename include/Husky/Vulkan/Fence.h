#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    enum class FenceWaitResult
    {
        Signaled,
        Timeout,
    };

    enum class FenceStatus
    {
        Signaled,
        Unsignaled
    };

    class Fence
    {
        friend class GraphicsDevice;
    public:
        Fence() = default;

        Fence(Fence&& other);
        Fence& operator=(Fence&& other);

        ~Fence();

        inline vk::Fence GetFence() { return fence; }

        VulkanResultValue<FenceWaitResult> Wait(Optional<Timeout> timeout = {});
        VulkanResultValue<FenceStatus> GetStatus();
        vk::Result Reset();
    private:
        Fence(GraphicsDevice* device, vk::Fence fence);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Fence fence;
    };
}
