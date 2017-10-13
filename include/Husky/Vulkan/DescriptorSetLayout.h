#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class DescriptorSetBinding
    {
    public:
        
    private:
        int32 index = -1;
    };

    class DescriptorSetLayoutCreateInfo
    {
    public:
        static vk::DescriptorSetLayoutCreateInfo ToVkCreateInfo(const DescriptorSetLayoutCreateInfo& ci);

        
    private:
    };

    class DescriptorSetLayout
    {
        friend class GraphicsDevice;
    public:
        DescriptorSetLayout() = default;

        DescriptorSetLayout(DescriptorSetLayout&& other);
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other);

        ~DescriptorSetLayout();

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        DescriptorSetLayout(GraphicsDevice* device, vk::DescriptorSetLayout descriptorSetLayout);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
