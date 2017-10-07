#include <Husky/Vulkan/Framebuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    FramebufferCreateInfo::FramebufferCreateInfo(RenderPass* aRenderPass, int32 aWidth, int32 aHeight, int32 aLayers)
        : renderPass(aRenderPass)
        , width(aWidth)
        , height(aHeight)
        , layers(aLayers)
    {
    }

    Framebuffer::Framebuffer(GraphicsDevice* aDevice, vk::Framebuffer aFramebuffer)
        : device(aDevice)
        , framebuffer(aFramebuffer)
    {
    }

    Framebuffer::Framebuffer(Framebuffer&& other)
        : device(other.device)
        , framebuffer(other.framebuffer)
    {
        other.device = nullptr;
        other.framebuffer = nullptr;
    }

    Framebuffer& Framebuffer::operator=(Framebuffer && other)
    {
        Destroy();

        device = other.device;
        framebuffer = other.framebuffer;

        other.device = nullptr;
        other.framebuffer = nullptr;

        return *this;
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

    vk::FramebufferCreateInfo FramebufferCreateInfo::ToVulkanCreateInfo(const FramebufferCreateInfo & createInfo)
    {
        vk::FramebufferCreateInfo vkci;
        vkci.setRenderPass(createInfo.renderPass->GetRenderPass());
        vkci.setAttachmentCount((int32)createInfo.attachments.size());
        vkci.setPAttachments(createInfo.attachments.data());
        vkci.setWidth(createInfo.width);
        vkci.setHeight(createInfo.height);
        vkci.setLayers(createInfo.layers);
        return vkci;
    }
}
