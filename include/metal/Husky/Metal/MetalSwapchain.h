#pragma once

#include <Husky/Graphics/Swapchain.h>
#include <Husky/Graphics/SwapchainInfo.h>
#include <Husky/Metal/MetalForwards.h>
#import <dispatch/dispatch.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalSwapchain : public Swapchain
    {
        friend class MetalCommandQueue;
    public:
        MetalSwapchain(
            MetalGraphicsDevice* device,
            const SwapchainInfo& swapchainInfo,
            CAMetalLayer* layer);

        inline const SwapchainInfo& GetInfo() const override { return swapchainInfo; }
        GraphicsResultValue<AcquiredTexture> GetNextAvailableTexture(Semaphore* semaphore) override;
    private:
        SwapchainInfo swapchainInfo;
        id<CAMetalDrawable> drawable;
        CAMetalLayer* layer = nil;
    };
}
