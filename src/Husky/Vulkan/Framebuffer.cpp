#include <Husky/Vulkan/Framebuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Framebuffer::Framebuffer(GraphicsDevice* aDevice, vk::Framebuffer aFramebuffer)
        : device(aDevice)
        , framebuffer(aFramebuffer)
    {
    }

    Framebuffer::~Framebuffer()
    {
        Destroy();
    }

    void Framebuffer::Destroy()
    {
        if (device)
        {
            device->DestroyFramebuffer(this);
        }
    }
}
