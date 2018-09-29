#pragma once

#include <Husky/Graphics/DescriptorSetLayout.h>
#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalDescriptorSetLayout : public DescriptorSetLayout
    {
        friend class MetalDescriptorPool;
        friend class MetalGraphicsCommandList;
    public:
        MetalDescriptorSetLayout(
            MetalGraphicsDevice* device,
            const DescriptorSetLayoutCreateInfo& createInfo);

        const DescriptorSetLayoutCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        DescriptorSetLayoutCreateInfo createInfo;
    };
}

