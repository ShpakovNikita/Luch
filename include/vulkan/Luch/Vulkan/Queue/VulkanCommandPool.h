#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/CommandPool.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanCommandPool : public CommandPool
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanCommandPool(
            VulkanGraphicsDevice* device,
            vk::CommandPool commandPool);
        ~VulkanCommandPool() override;

        GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() override;
        GraphicsResultRefPtr<CopyCommandList> AllocateCopyCommandList() override;
        GraphicsResultRefPtr<ComputeCommandList> AllocateComputeCommandList() override;

        inline vk::CommandPool GetCommandPool() { return commandPool; }
        vk::Result Reset(bool releaseResources = false);

    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::CommandPool commandPool;
    };
}
