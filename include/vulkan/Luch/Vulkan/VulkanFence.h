#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>

using namespace Luch::Graphics;

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

        GraphicsResultValue<FenceWaitResult> Wait(Optional<Timeout> timeout = {});
        GraphicsResultValue<FenceStatus> GetStatus();
        vk::Result Reset();
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Fence fence;
    };
}
