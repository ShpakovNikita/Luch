#include <Luch/Metal/MetalRenderPass.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
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
