#include <Husky/Vulkan/RenderPass.h.>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    RenderPass::RenderPass(GraphicsDevice* aDevice, vk::RenderPass aRenderPass)
        : device(aDevice)
        , renderPass(aRenderPass)
    {
    }

    RenderPass::RenderPass(RenderPass && other)
        : device(other.device)
        , renderPass(other.renderPass)
    {
        other.device = nullptr;
        other.renderPass = nullptr;
    }

    RenderPass & RenderPass::operator=(RenderPass && other)
    {
        device = other.device;
        renderPass = other.renderPass;

        other.device = nullptr;
        other.renderPass = nullptr;

        return *this;
    }

    RenderPass::~RenderPass()
    {
        if (device)
        {
            device->DestroyRenderPass(this);
        }
    }

    RenderPassCreateInfo::VulkanRenderPassCreateInfo RenderPassCreateInfo::ToVulkanCreateInfo(const RenderPassCreateInfo & createInfo)
    {
        VulkanRenderPassCreateInfo result;
        auto& vkci = result.createInfo;
        vkci.setAttachmentCount((int32)createInfo.attachments.size());
        return result;
    }
}