#pragma once

#include <Husky/Graphics/Swapchain.h>
#include <Husky/Graphics/SwapchainCreateInfo.h>
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
            const SwapchainCreateInfo& createInfo,
            CAMetalLayer* layer);

        inline const SwapchainCreateInfo& GetCreateInfo() const override { return createInfo; }
        GraphicsResultValue<AcquiredTexture> GetNextAvailableTexture(Semaphore* semaphore) override;
    private:
        SwapchainCreateInfo createInfo;
        id<CAMetalDrawable> drawable;
        CAMetalLayer* layer = nil;
    };
}
