#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class Framebuffer : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Framebuffer(GraphicsDevice* device, vk::Framebuffer framebuffer);
        ~Framebuffer();

        vk::Framebuffer GetFramebuffer() { return framebuffer; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Framebuffer framebuffer;
    };
}
