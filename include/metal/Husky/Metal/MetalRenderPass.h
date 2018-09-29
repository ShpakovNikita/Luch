#pragma once

#include <Husky/Graphics/RenderPass.h>
#include <Husky/Graphics/RenderPassCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
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
