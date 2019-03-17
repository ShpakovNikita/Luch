#pragma once

#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Metal/MetalForwards.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalSwapchainTexture : public SwapchainTexture
    {
        friend class MetalCommandQueue;
    public:
        MetalSwapchainTexture(RefPtr<Texture> texture, mtlpp::Drawable drawable);

        RefPtr<Texture> GetTexture() { return texture; }
    private:
        RefPtr<Texture> texture;
        mtlpp::Drawable drawable;
    };

    class MetalSwapchain : public Swapchain
    {
        friend class MetalCommandQueue;
    public:
        MetalSwapchain(
            MetalGraphicsDevice* device,
            const SwapchainInfo& swapchainInfo,
            CAMetalLayer* layer);

        inline const SwapchainInfo& GetInfo() const override { return swapchainInfo; }
        GraphicsResultRefPtr<SwapchainTexture> GetNextAvailableTexture() override;
    private:
        SwapchainInfo swapchainInfo;
        CAMetalLayer* layer = nil;
    };
}
