#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
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

    class Fence : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Fence(GraphicsDevice* device, vk::Fence fence);
        ~Fence() override;

        inline vk::Fence GetFence() { return fence; }

        VulkanResultValue<FenceWaitResult> Wait(Optional<Timeout> timeout = {});
        VulkanResultValue<FenceStatus> GetStatus();
        vk::Result Reset();
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Fence fence;
    };
}
