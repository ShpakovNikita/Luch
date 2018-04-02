#include <Husky/Vulkan/RenderPass.h.>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/Vulkan/SampleCount.h>

namespace Husky::Vulkan
{
    RenderPass::RenderPass(
        GraphicsDevice* aDevice,
        vk::RenderPass aRenderPass,
        int32 aAttachmentCount)
        : device(aDevice)
        , renderPass(aRenderPass)
        , attachmentCount(aAttachmentCount)
    {
    }

    RenderPass::~RenderPass()
    {
        Destroy();
    }

    void RenderPass::Destroy()
    {
        if (device)
        {
            device->DestroyRenderPass(this);
        }
    }
}
