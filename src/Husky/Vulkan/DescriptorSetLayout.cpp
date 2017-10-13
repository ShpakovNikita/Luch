#include <Husky/Vulkan/DescriptorSetLayout.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    DescriptorSetLayout::DescriptorSetLayout(GraphicsDevice* aDevice, vk::DescriptorSetLayout aDescriptorSetLayout)
        : device(aDevice)
        , descriptorSetLayout(aDescriptorSetLayout)
    {
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other)
        : device(other.device)
        , descriptorSetLayout(other.descriptorSetLayout)
    {
        other.device = nullptr;
        other.descriptorSetLayout = nullptr;
    }

    DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other)
    {
        Destroy();

        device = other.device;
        descriptorSetLayout = other.descriptorSetLayout;

        other.device = nullptr;
        other.descriptorSetLayout = nullptr;

        return *this;
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        Destroy();
    }

    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo::ToVkCreateInfo(const DescriptorSetLayoutCreateInfo& ci)
    {
        vk::DescriptorSetLayoutCreateInfo vkci;
        //vkci.setBindingCount((uint32)ci.bindings.size());
        //vkci.setPBindings(ci.bindings.data());
        return vkci;
    }

    void DescriptorSetLayout::Destroy()
    {
        if (device)
        {
            device->DestroyDescriptorSetLayout(this);
        }
    }
}