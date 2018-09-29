#include <Husky/Metal/MetalSwapchain.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalSemaphore.h>
#include <Husky/Metal/MetalTexture.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalSwapchain::MetalSwapchain(
        MetalGraphicsDevice* device,
        const SwapchainCreateInfo& aCreateInfo,
        CAMetalLayer* aLayer)
        : Swapchain(device)
        , createInfo(aCreateInfo)
        , layer(aLayer)
    {
        HUSKY_ASSERT(createInfo.imageCount == 1);
        //frameSemaphore = dispatch_semaphore_create(createInfo.imageCount);
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());

        layer.device = (__bridge id<MTLDevice>)mtlDevice->device.GetPtr();
    }

//    GraphicsResultValue<AcquiredTexture> MetalSwapchain::GetNextAvailableTexture(
//        Semaphore* semaphore)
//    {
//        auto mtlSemaphore = static_cast<MetalSemaphore*>(semaphore);
//        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
//
//        dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);
//
//        auto acquiredIndex = currentImageIndex;
//        currentImageIndex = (currentImageIndex + 1) % createInfo.imageCount;
//
//        TextureCreateInfo textureCreateInfo;
//        // TODO
//        id<CAMetalDrawable> drawable = [layer nextDrawable];
//        drawables[acquiredIndex] = drawable;
//        mtlpp::Texture mtlTexture = ns::Handle{ (__bridge void*)drawable.texture };
//
//        dispatch_semaphore_signal(mtlSemaphore->semaphore);
//
//        auto texture = MakeRef<MetalTexture>(mtlDevice, textureCreateInfo, mtlTexture);
//        return { GraphicsResult::Success, { acquiredIndex, texture } };
//    }

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
