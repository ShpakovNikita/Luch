#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class DescriptorSetLayoutCreateInfo;

    // TODO propagate index from pipeline layout
    class DescriptorSetLayout : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        DescriptorSetLayout(GraphicsDevice* device, vk::DescriptorSetLayout descriptorSetLayout);

        DescriptorSetLayout(const DescriptorSetLayout& other) = delete;
        DescriptorSetLayout(DescriptorSetLayout&& other) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout& other) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other) = delete;

        ~DescriptorSetLayout() override;

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
