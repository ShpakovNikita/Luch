#pragma once

#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalDescriptorSetLayout : public DescriptorSetLayout
    {
        friend class MetalDescriptorPool;
        friend class MetalGraphicsCommandList;
        friend class MetalComputeCommandList;
    public:
        MetalDescriptorSetLayout(
            MetalGraphicsDevice* device,
            const DescriptorSetLayoutCreateInfo& createInfo);

        const DescriptorSetLayoutCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        DescriptorSetLayoutCreateInfo createInfo;
    };
}

