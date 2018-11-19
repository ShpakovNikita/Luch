#include <Luch/Metal/MetalSwapchain.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalTexture.h>

namespace Luch::Metal
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
        LUCH_ASSERT(swapchainInfo.imageCount == 1);
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
