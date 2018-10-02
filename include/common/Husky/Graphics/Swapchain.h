#pragma once

#include <Husky/Graphics/GraphicsObject.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    struct AcquiredTexture
    {
        int32 index = 0;
        RefPtr<Texture> texture;
    };

    class Swapchain : public GraphicsObject
    {
    public:
        Swapchain(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Swapchain() = 0;

        virtual const SwapchainInfo& GetInfo() const = 0;
        virtual GraphicsResultValue<AcquiredTexture> GetNextAvailableTexture(Semaphore* semaphore) = 0;
    };

    inline Swapchain::~Swapchain() {}
}
