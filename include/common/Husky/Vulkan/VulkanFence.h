#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

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
