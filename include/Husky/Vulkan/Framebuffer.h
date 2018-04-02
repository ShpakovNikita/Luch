#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class RenderPass;

    class Framebuffer : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Framebuffer(GraphicsDevice* device, vk::Framebuffer framebuffer);

        Framebuffer(const Framebuffer& other) = delete;
        Framebuffer(Framebuffer&& other) = delete;
        Framebuffer& operator=(const Framebuffer& other) = delete;
        Framebuffer& operator=(Framebuffer&& other) = delete;

        ~Framebuffer();

        vk::Framebuffer GetFramebuffer() { return framebuffer; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Framebuffer framebuffer;
    };
}
