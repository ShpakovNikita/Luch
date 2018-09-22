#pragma once

#include <Husky/Graphics/ShaderStage.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    inline vk::ShaderStageFlags ToVulkanShaderStages(ShaderStage stages)
    {
        vk::ShaderStageFlags vulkanStages;
        if ((stages & ShaderStage::Fragment) == ShaderStage::Fragment)
        {
            vulkanStages |= vk::ShaderStageFlagBits::eFragment;
        }
        if ((stages & ShaderStage::Geometry) == ShaderStage::Geometry)
        {
            vulkanStages |= vk::ShaderStageFlagBits::eGeometry;
        }
        if ((stages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
        {
            vulkanStages |= vk::ShaderStageFlagBits::eTessellationControl;
        }
        if ((stages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
        {
            vulkanStages |= vk::ShaderStageFlagBits::eTessellationEvaluation;
        }
        if ((stages & ShaderStage::Vertex) == ShaderStage::Vertex)
        {
            vulkanStages |= vk::ShaderStageFlagBits::eVertex;
        }
        if ((stages & ShaderStage::Compute) == ShaderStage::Compute)
        {
            vulkanStages |= vk::ShaderStageFlagBits::eCompute;
        }

        return vulkanStages;
    }

    inline vk::ShaderStageFlagBits ToVulkanShaderStage(ShaderStage stages)
    {
        // TODO assert only one bit is set
        if ((stages & ShaderStage::Fragment) == ShaderStage::Fragment)
        {
            return vk::ShaderStageFlagBits::eFragment;
        }
        if ((stages & ShaderStage::Geometry) == ShaderStage::Geometry)
        {
            return vk::ShaderStageFlagBits::eGeometry;
        }
        if ((stages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
        {
            return vk::ShaderStageFlagBits::eTessellationControl;
        }
        if ((stages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
        {
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
        }
        if ((stages & ShaderStage::Vertex) == ShaderStage::Vertex)
        {
            return vk::ShaderStageFlagBits::eVertex;
        }
        if ((stages & ShaderStage::Compute) == ShaderStage::Compute)
        {
            return vk::ShaderStageFlagBits::eCompute;
        }

        HUSKY_ASSERT_MSG(false, "Unknown shader stage");
        return (vk::ShaderStageFlagBits)0;
    }
}
