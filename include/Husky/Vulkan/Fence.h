#pragma once

#include <Husky/BaseObject.h>
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

    class Fence : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Fence(GraphicsDevice* device, vk::Fence fence);

        Fence(const Fence& other) = delete;
        Fence(Fence&& other) = delete;
        Fence& operator=(const Fence& other) = delete;
        Fence& operator=(Fence&& other) = delete;

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
