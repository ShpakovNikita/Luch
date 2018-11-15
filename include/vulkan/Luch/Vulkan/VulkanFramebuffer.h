#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
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
