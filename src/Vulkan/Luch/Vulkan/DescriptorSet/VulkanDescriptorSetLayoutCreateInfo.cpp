#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSetLayoutCreateInfo.h>
#include <Luch/Vulkan/Shader/VulkanShaderStage.h>

namespace Luch::Vulkan
{
    vk::DescriptorType ToVulkanDescriptorType(DescriptorSetType type)
    {
        switch(type)
        {
        case DescriptorSetType::Texture:
            return vk::DescriptorType::eStorageImage;
        case DescriptorSetType::Buffer:
            return vk::DescriptorType::eUniformBuffer;
        case DescriptorSetType::Sampler:
            return vk::DescriptorType::eCombinedImageSampler;
        case DescriptorSetType::Unknown:
            LUCH_ASSERT_MSG(false, "Unknown DescriptorSetType");
            return vk::DescriptorType::eUniformBuffer;
        }
    }

    VulkanDescriptorSetLayoutCreateInfo
    VulkanDescriptorSetLayoutCreateInfo::ToVulkanCreateInfo(const DescriptorSetLayoutCreateInfo& ci)
    {
        VulkanDescriptorSetLayoutCreateInfo description;
        description.bindings.reserve(ci.bindings.size());
        vk::DescriptorType type = ToVulkanDescriptorType(ci.type);

        uint32_t currentBindingCount = 0;
        for (auto& binding : ci.bindings)
        {
            uint32_t descriptorCount = 1;
            auto& vulkanBinding = description.bindings.emplace_back();
            vulkanBinding.setBinding(currentBindingCount);
            vulkanBinding.setDescriptorCount(descriptorCount);
            vulkanBinding.setDescriptorType(type);
            vulkanBinding.setStageFlags(
                        ToVulkanShaderStages(ShaderStage::Vertex | ShaderStage::Fragment | ShaderStage::Compute)
                        );

            // todo: figure out what's this samplers code
            /*const auto& immutableSamplers = binding->GetImmutableSamplers();
            if (!immutableSamplers.empty())
            {
                LUCH_ASSERT_MSG(immutableSamplers.size() == descriptorCount, "Immutable samplers count must match descriptor count");
                auto& samplers = description.samplers.emplace_back();
                samplers.reserve(descriptorCount);
                for (auto& sampler : immutableSamplers)
                {
                    samplers.push_back(sampler);
                }
                vulkanBinding.setPImmutableSamplers(samplers.data());
            }*/

            currentBindingCount += descriptorCount;
        }

        description.createInfo.setPBindings(description.bindings.data());
        description.createInfo.setBindingCount((int32)description.bindings.size());

        return description;
    }
}
