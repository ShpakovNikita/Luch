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

    DescriptorSetLayoutCreateInfo::VulkanDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo::ToVkCreateInfo(const DescriptorSetLayoutCreateInfo& ci)
    {
        VulkanDescriptorSetLayoutCreateInfo description;
        description.bindings.resize(ci.bindings.size());

        int currentBindingCount = 0;
        for (auto& binding : ci.bindings)
        {
            vk::DescriptorSetLayoutBinding vulkanBinding;
            vulkanBinding.setBinding(currentBindingCount);
            vulkanBinding.setDescriptorCount(binding->count);
            vulkanBinding.setDescriptorType(binding->type);
            vulkanBinding.setStageFlags(binding->stages);

            if (!binding->immutableSamplers.empty())
            {
                HUSKY_ASSERT(binding->immutableSamplers.size() == binding->count, "Immutable samplers count must match descriptor count");
                auto& samplers = description.samplers.emplace_back();
                samplers.reserve(binding->count);
                for (auto& sampler : binding->immutableSamplers)
                {
                    samplers.push_back(sampler);
                }
                vulkanBinding.setPImmutableSamplers(samplers.data());
            }

            description.bindings.push_back(vulkanBinding);
            currentBindingCount += binding->count;
        }

        description.createInfo.setPBindings(description.bindings.data());
        description.createInfo.setBindingCount((int32)description.bindings.size());

        return description;
    }

    void DescriptorSetLayout::Destroy()
    {
        if (device)
        {
            device->DestroyDescriptorSetLayout(this);
        }
    }
}
