#include <Husky/Vulkan/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Vulkan/ShaderStage.h>

namespace Husky::Vulkan
{
    DescriptorSetLayoutCreateInfo::VulkanDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo::ToVulkanCreateInfo(const DescriptorSetLayoutCreateInfo& ci)
    {
        VulkanDescriptorSetLayoutCreateInfo description;
        description.bindings.reserve(ci.bindings.size());

        int currentBindingCount = 0;
        for (auto& binding : ci.bindings)
        {
            auto& vulkanBinding = description.bindings.emplace_back();
            vulkanBinding.setBinding(currentBindingCount);
            vulkanBinding.setDescriptorCount(binding->count);
            vulkanBinding.setDescriptorType(binding->type);
            vulkanBinding.setStageFlags(ToVulkanShaderStages(binding->stages));

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

            currentBindingCount += binding->count;
        }

        description.createInfo.setPBindings(description.bindings.data());
        description.createInfo.setBindingCount((int32)description.bindings.size());

        return description;
    }
}