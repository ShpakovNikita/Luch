#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
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

    class VulkanFence : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanFence(VulkanGraphicsDevice* device, vk::Fence fence);
        ~VulkanFence() override;

        inline vk::Fence GetFence() { return fence; }

        VulkanResultValue<FenceWaitResult> Wait(Optional<Timeout> timeout = {});
        VulkanResultValue<FenceStatus> GetStatus();
        vk::Result Reset();
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Fence fence;
    };
}
