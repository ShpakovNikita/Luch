#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanPipelineLayout : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanPipelineLayout(VulkanGraphicsDevice* device, vk::PipelineLayout pipelineLayout);
        ~VulkanPipelineLayout() override;

        inline vk::PipelineLayout GetPipelineLayout() { return pipelineLayout; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::PipelineLayout pipelineLayout;
    };
}
