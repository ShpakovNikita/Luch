#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class SwapchainTexture : public BaseObject
    {
    public:
        virtual ~SwapchainTexture() = 0;
        virtual RefPtr<Texture> GetTexture() = 0;
    };

    inline SwapchainTexture::~SwapchainTexture() = default;

    class Swapchain : public GraphicsObject
    {
    public:
        Swapchain(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~Swapchain() = 0;

        virtual const SwapchainInfo& GetInfo() const = 0;
        virtual GraphicsResultRefPtr<SwapchainTexture> GetNextAvailableTexture() = 0;
    };

    inline Swapchain::~Swapchain() {}
}
