#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    PipelineLayout::PipelineLayout(GraphicsDevice* aDevice, vk::PipelineLayout aPipelineLayout)
        : device(aDevice)
        , pipelineLayout(aPipelineLayout)
    {
    }

    PipelineLayout::PipelineLayout(PipelineLayout&& other)
        : device(other.device)
        , pipelineLayout(other.pipelineLayout)
    {
        other.device = nullptr;
        other.pipelineLayout = nullptr;
    }

    PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other)
    {
        device = other.device;
        pipelineLayout = other.pipelineLayout;

        other.device = nullptr;
        other.pipelineLayout = nullptr;

        return *this;
    }

    PipelineLayout::~PipelineLayout()
    {
        Destroy();
    }

    void PipelineLayout::Destroy()
    {
        if (device)
        {
            device->DestroyPipelineLayout(this);
        }
    }

    PipelineLayoutCreateInfo::VulkanPipelineLayoutCreateInfo PipelineLayoutCreateInfo::ToVkCreateInfo(const PipelineLayoutCreateInfo& ci)
    {
        VulkanPipelineLayoutCreateInfo createInfo;
        createInfo.setLayouts.reserve(ci.setLayouts.size());
        for(auto setLayout : ci.setLayouts)
        {
            createInfo.setLayouts.push_back(setLayout->GetDescriptorSetLayout());
        }

        createInfo.createInfo.setPSetLayouts(createInfo.setLayouts.data());
        createInfo.createInfo.setSetLayoutCount((int32)createInfo.setLayouts.size());

        return createInfo;
    }
}
