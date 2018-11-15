#pragma once

#include <Luch/Graphics/RenderPass.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;
    class MetalRenderPass : public RenderPass
    {
    public:
        MetalRenderPass(
            MetalGraphicsDevice* device,
            const RenderPassCreateInfo& createInfo);

        const RenderPassCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        RenderPassCreateInfo createInfo;
    };
}
