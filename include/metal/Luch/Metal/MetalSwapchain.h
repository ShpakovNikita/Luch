#pragma once

#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Metal/MetalForwards.h>
#import <dispatch/dispatch.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Luch::Metal
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
