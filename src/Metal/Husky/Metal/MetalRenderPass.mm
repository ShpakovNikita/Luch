#include <Husky/Metal/MetalRenderPass.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalRenderPass::MetalRenderPass(
        MetalGraphicsDevice* device,
        const RenderPassCreateInfo& aCreateInfo)
        : RenderPass(device)
        , createInfo(aCreateInfo)
    {
    }
}
