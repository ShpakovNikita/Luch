#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    // TODO propagate index from pipeline layout
    class DescriptorSetLayout : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        DescriptorSetLayout(GraphicsDevice* device, vk::DescriptorSetLayout descriptorSetLayout);
        ~DescriptorSetLayout() override;

        vk::DescriptorSetLayout GetDescriptorSetLayout() { return descriptorSetLayout; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::DescriptorSetLayout descriptorSetLayout;
    };
}
