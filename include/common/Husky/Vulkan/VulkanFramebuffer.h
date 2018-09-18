#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class VulkanFramebuffer : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanFramebuffer(VulkanGraphicsDevice* device, vk::Framebuffer framebuffer);
        ~VulkanFramebuffer();

        vk::Framebuffer GetFramebuffer() { return framebuffer; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Framebuffer framebuffer;
    };
}
