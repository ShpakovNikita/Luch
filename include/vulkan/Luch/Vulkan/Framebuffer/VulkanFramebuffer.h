#pragma once

#include <vulkan/vulkan.hpp>
#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/FrameBuffer.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanFramebuffer : public FrameBuffer
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanFramebuffer(VulkanGraphicsDevice* device, vk::Framebuffer framebuffer);
        ~VulkanFramebuffer();

        const FrameBufferCreateInfo& GetCreateInfo() const override
        {
            return unused;
        }

        vk::Framebuffer GetFramebuffer() { return framebuffer; }
    private:
        void Destroy();

        FrameBufferCreateInfo unused;
        VulkanGraphicsDevice* device = nullptr;
        vk::Framebuffer framebuffer;
    };
}
