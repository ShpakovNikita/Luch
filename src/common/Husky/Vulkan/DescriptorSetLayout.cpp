#include <Husky/Vulkan/DescriptorSetLayout.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/ShaderStage.h>

namespace Husky::Vulkan
{
    DescriptorSetLayout::DescriptorSetLayout(GraphicsDevice* aDevice, vk::DescriptorSetLayout aDescriptorSetLayout)
        : device(aDevice)
        , descriptorSetLayout(aDescriptorSetLayout)
    {
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        Destroy();
    }

    void DescriptorSetLayout::Destroy()
    {
        if (device)
        {
            device->DestroyDescriptorSetLayout(this);
        }
    }
}
