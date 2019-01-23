#include <Luch/Metal/MetalSwapchain.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalTexture.h>

namespace Luch::Metal
{
    using namespace Graphics;

    MetalSwapchainTexture::MetalSwapchainTexture(
        RefPtr<Texture> aTexture,
        mtlpp::Drawable aDrawable)
        : texture(aTexture)
        , drawable(aDrawable)
    {
    }

    MetalSwapchain::MetalSwapchain(
        MetalGraphicsDevice* device,
        const SwapchainInfo& aCreateInfo,
        CAMetalLayer* aLayer)
        : Swapchain(device)
        , swapchainInfo(aCreateInfo)
        , layer(aLayer)
    {
    }

    GraphicsResultRefPtr<SwapchainTexture> MetalSwapchain::GetNextAvailableTexture()
    {
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());

        TextureCreateInfo textureCreateInfo;

        id<CAMetalDrawable> drawable = [layer nextDrawable];
        mtlpp::Drawable mtlDrawable = ns::Handle{ (__bridge void*)drawable };
        mtlpp::Texture mtlTexture = ns::Handle{ (__bridge void*)drawable.texture };

        auto texture = MakeRef<MetalTexture>(mtlDevice, textureCreateInfo, mtlTexture);
        auto swapchainTexture = MakeRef<MetalSwapchainTexture>(texture, mtlDrawable);

        return { GraphicsResult::Success, swapchainTexture };
    }
}
