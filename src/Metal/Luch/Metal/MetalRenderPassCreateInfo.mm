#include <Luch/Metal/MetalRenderPassCreateInfo.h>
#include <Luch/Graphics/RenderPass.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>
#include <Luch/Metal/MetalTexture.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::LoadAction ToMetalLoadAction(AttachmentLoadOperation loadOperation)
    {
        switch(loadOperation)
        {
        case AttachmentLoadOperation::Clear:
            return mtlpp::LoadAction::Clear;
        case AttachmentLoadOperation::DontCare:
            return mtlpp::LoadAction::DontCare;
        case AttachmentLoadOperation::Load:
            return mtlpp::LoadAction::Load;
        default:
            LUCH_ASSERT_MSG(false, "Unknown load operation");
            return mtlpp::LoadAction::DontCare;
        }
    }

    mtlpp::StoreAction ToMetalStoreAction(AttachmentStoreOperation storeOperation)
    {
        switch(storeOperation)
        {
        case Graphics::AttachmentStoreOperation::DontCare:
            return mtlpp::StoreAction::DontCare;
        case Graphics::AttachmentStoreOperation::Store:
            return mtlpp::StoreAction::Store;
        default:
            LUCH_ASSERT_MSG(false, "Unknown store operation");
            return mtlpp::StoreAction::DontCare;
        }
    }

    mtlpp::RenderPassDescriptor ToMetalRenderPassDescriptor(const FrameBufferCreateInfo& frameBufferCreateInfo)
    {
        mtlpp::RenderPassDescriptor d;
        const auto& renderPassCreateInfo = frameBufferCreateInfo.renderPass->GetCreateInfo();
        LUCH_ASSERT(renderPassCreateInfo.colorAttachments.size() == frameBufferCreateInfo.colorTextures.size());
        for(size_t i = 0; i < renderPassCreateInfo.colorAttachments.size(); i++)
        {
            const auto& colorAttachment = renderPassCreateInfo.colorAttachments[i];
            if(colorAttachment.has_value())
            {
                auto mtlTexture = static_cast<MetalTexture*>(frameBufferCreateInfo.colorTextures[i]);
                LUCH_ASSERT(mtlTexture != nullptr);

                mtlpp::ClearColor clearColor = {
                    colorAttachment->clearValue.red,
                    colorAttachment->clearValue.green,
                    colorAttachment->clearValue.blue,
                    colorAttachment->clearValue.alpha
                };
                d.GetColorAttachments()[i].SetTexture(mtlTexture->GetNativeTexture());
                d.GetColorAttachments()[i].SetLevel(colorAttachment->output.mipmapLevel);
                d.GetColorAttachments()[i].SetSlice(colorAttachment->output.slice);
                d.GetColorAttachments()[i].SetDepthPlane(colorAttachment->output.depthPlane);
                d.GetColorAttachments()[i].SetClearColor(clearColor);
                d.GetColorAttachments()[i].SetLoadAction(ToMetalLoadAction(colorAttachment->colorLoadOperation));
                d.GetColorAttachments()[i].SetStoreAction(ToMetalStoreAction(colorAttachment->colorStoreOperation));
            }
        }

        if(renderPassCreateInfo.depthStencilAttachment.has_value())
        {
            const auto& depthStencilAttachment = renderPassCreateInfo.depthStencilAttachment;
            auto mtlDepthStencilBuffer = static_cast<MetalTexture*>(frameBufferCreateInfo.depthStencilTexture);
            LUCH_ASSERT(mtlDepthStencilBuffer != nullptr);

            if(FormatHasDepth(depthStencilAttachment->format))
            {
                d.GetDepthAttachment().SetTexture(mtlDepthStencilBuffer->GetNativeTexture());
                d.GetDepthAttachment().SetLevel(depthStencilAttachment->output.mipmapLevel);
                d.GetDepthAttachment().SetSlice(depthStencilAttachment->output.slice);
                d.GetDepthAttachment().SetDepthPlane(depthStencilAttachment->output.depthPlane);
                d.GetDepthAttachment().SetClearDepth(depthStencilAttachment->depthClearValue);
                d.GetDepthAttachment().SetLoadAction(ToMetalLoadAction(depthStencilAttachment->depthLoadOperation));
                d.GetDepthAttachment().SetStoreAction(ToMetalStoreAction(depthStencilAttachment->depthStoreOperation));
            }

            if(FormatHasStencil(depthStencilAttachment->format))
            {
                d.GetStencilAttachment().SetTexture(mtlDepthStencilBuffer->GetNativeTexture());
                d.GetStencilAttachment().SetLevel(depthStencilAttachment->output.mipmapLevel);
                d.GetStencilAttachment().SetSlice(depthStencilAttachment->output.slice);
                d.GetStencilAttachment().SetDepthPlane(depthStencilAttachment->output.depthPlane);
                d.GetStencilAttachment().SetClearStencil(depthStencilAttachment->stencilClearValue);
                d.GetStencilAttachment().SetLoadAction(ToMetalLoadAction(depthStencilAttachment->stencilLoadOperation));
                d.GetStencilAttachment().SetStoreAction(ToMetalStoreAction(depthStencilAttachment->stencilStoreOperation));
            }
        }

        return d;
    }
}
