#pragma once

#include <Husky/Graphics/PipelineLayout.h>
#include <Husky/Graphics/PipelineLayoutCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalPipelineLayout : public PipelineLayout
    {
        friend class MetalGraphicsCommandList;
    public:
        MetalPipelineLayout(
            MetalGraphicsDevice* device,
            const PipelineLayoutCreateInfo& createInfo);

        const PipelineLayoutCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        PipelineLayoutCreateInfo createInfo;
    };
}
