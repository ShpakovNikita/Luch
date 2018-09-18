#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
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
