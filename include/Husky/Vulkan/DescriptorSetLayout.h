#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class DescriptorSetLayoutCreateInfo
    {
    public:
        static vk::DescriptorSetLayoutCreateInfo ToVkCreateInfo(const DescriptorSetLayoutCreateInfo& ci);

        inline DescriptorSetLayoutCreateInfo& AddBinding(const vk::DescriptorSetLayoutBinding& binding)
        {
            bindings.push_back(binding);
            return *this;
        }
    private:
        Vector<vk::DescriptorSetLayoutBinding> bindings;
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
