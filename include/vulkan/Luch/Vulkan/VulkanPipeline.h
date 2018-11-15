#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanPipeline : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanPipeline(VulkanGraphicsDevice* device, vk::Pipeline pipeline);
        ~VulkanPipeline() override;

        vk::Pipeline GetPipeline() { return pipeline; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Pipeline pipeline;
    };
}
