#include <Luch/Vulkan/VulkanDescriptorSetLayoutCreateInfo.h>
#include <Luch/Vulkan/VulkanShaderStage.h>

namespace Luch::Vulkan
{
    DescriptorSetLayoutCreateInfo::VulkanDescriptorSetLayoutCreateInfo
    DescriptorSetLayoutCreateInfo::ToVulkanCreateInfo(const DescriptorSetLayoutCreateInfo& ci)
    {
        VulkanDescriptorSetLayoutCreateInfo description;
        description.bindings.reserve(ci.bindings.size());

        int currentBindingCount = 0;
        for (auto& binding : ci.bindings)
        {
            auto& vulkanBinding = description.bindings.emplace_back();
            vulkanBinding.setBinding(currentBindingCount);
            vulkanBinding.setDescriptorCount(binding->GetCount());
            vulkanBinding.setDescriptorType(binding->GetType());
            vulkanBinding.setStageFlags(ToVulkanShaderStages(binding->GetStages()));

            const auto& immutableSamplers = binding->GetImmutableSamplers();
            if (!immutableSamplers.empty())
            {
                LUCH_ASSERT_MSG(immutableSamplers.size() == binding->GetCount(), "Immutable samplers count must match descriptor count");
                auto& samplers = description.samplers.emplace_back();
                samplers.reserve(binding->GetCount());
                for (auto& sampler : immutableSamplers)
                {
                    samplers.push_back(sampler);
                }
                vulkanBinding.setPImmutableSamplers(samplers.data());
            }

            currentBindingCount += binding->GetCount();
        }

        description.createInfo.setPBindings(description.bindings.data());
        description.createInfo.setBindingCount((int32)description.bindings.size());

        return description;
    }

    DescriptorSetLayoutCreateInfo::VulkanDescriptorSetLayoutCreateInfo
    DescriptorSetLayoutCreateInfo::ToVulkanCreateInfo(const Luch::Graphics::DescriptorSetLayoutCreateInfo& ci)
    {
        VulkanDescriptorSetLayoutCreateInfo description;
        // todo: implement
        return description;
    }
}
