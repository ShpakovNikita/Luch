#include <Husky/Metal/MetalSwapchain.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalSemaphore.h>
#include <Husky/Metal/MetalTexture.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalSwapchain::MetalSwapchain(
        MetalGraphicsDevice* device,
        const SwapchainInfo& aCreateInfo,
        CAMetalLayer* aLayer)
        : Swapchain(device)
        , swapchainInfo(aCreateInfo)
        , layer(aLayer)
    {
        HUSKY_ASSERT(swapchainInfo.imageCount == 1);
        //frameSemaphore = dispatch_semaphore_create(createInfo.imageCount);
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());

        layer.device = (__bridge id<MTLDevice>)mtlDevice->device.GetPtr();
    }

    GraphicsResultValue<AcquiredTexture> MetalSwapchain::GetNextAvailableTexture(
        Semaphore* semaphore)
    {
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());

        TextureCreateInfo textureCreateInfo;
        // TODO
        drawable = [layer nextDrawable];
        mtlpp::Texture mtlTexture = ns::Handle{ (__bridge void*)drawable.texture };

        auto texture = MakeRef<MetalTexture>(mtlDevice, textureCreateInfo, mtlTexture);
        return { GraphicsResult::Success, { 0, texture } };
    }
}
