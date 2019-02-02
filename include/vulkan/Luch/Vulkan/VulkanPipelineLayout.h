#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>
#include <Luch/Graphics/PipelineLayout.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanPipelineLayout : public PipelineLayout
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanPipelineLayout(VulkanGraphicsDevice* device, vk::PipelineLayout vkPipelineLayout);
        ~VulkanPipelineLayout() override;

        const Graphics::PipelineLayoutCreateInfo& GetCreateInfo() const  override { return unusedCI; }
        inline vk::PipelineLayout GetPipelineLayout() { return vkPipelineLayout; }
    private:
        void Destroy();

        Graphics::PipelineLayoutCreateInfo unusedCI;
        VulkanGraphicsDevice* device = nullptr;
        vk::PipelineLayout vkPipelineLayout;
    };
}
