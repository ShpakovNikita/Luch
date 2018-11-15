#pragma once

#include <Luch/Graphics/PipelineLayout.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
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
