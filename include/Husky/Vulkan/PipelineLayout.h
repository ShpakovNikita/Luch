#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/DescriptorSetLayout.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class PipelineLayoutCreateInfo
    {
    public:
        struct VulkanPipelineLayoutCreateInfo
        {
            VulkanPipelineLayoutCreateInfo() = default;

            VulkanPipelineLayoutCreateInfo(const VulkanPipelineLayoutCreateInfo& other) = delete;
            VulkanPipelineLayoutCreateInfo& operator=(const VulkanPipelineLayoutCreateInfo& other) = delete;

            VulkanPipelineLayoutCreateInfo(VulkanPipelineLayoutCreateInfo&& other) = default;
            VulkanPipelineLayoutCreateInfo& operator=(VulkanPipelineLayoutCreateInfo&& other) = default;

            vk::PipelineLayoutCreateInfo createInfo;
            Vector<vk::DescriptorSetLayout> setLayouts;
        };

        static VulkanPipelineLayoutCreateInfo ToVulkanCreateInfo(const PipelineLayoutCreateInfo& ci);

        inline PipelineLayoutCreateInfo& WithNSetLayouts(int32 count)
        {
            setLayouts.reserve(count);
            return *this;
        }

        inline PipelineLayoutCreateInfo& AddSetLayout(DescriptorSetLayout* setLayout)
        {
            setLayouts.push_back(setLayout);
            return *this;
        }

        // TODO push constants
    private:
        Vector<DescriptorSetLayout*> setLayouts;
    };

    class PipelineLayout
    {
        friend class GraphicsDevice;
    public:
        PipelineLayout() = default;

        PipelineLayout(PipelineLayout&& other);
        PipelineLayout& operator=(PipelineLayout&& other);

        ~PipelineLayout();

        inline vk::PipelineLayout GetPipelineLayout() { return pipelineLayout; }
    private:
        PipelineLayout(GraphicsDevice* device, vk::PipelineLayout pipelineLayout);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::PipelineLayout pipelineLayout;
    };
}
