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
        LUCH_ASSERT(renderPassCreateInfo.colorAttachments.size() == frameBufferCreateInfo.colorAttachments.size());
        for(size_t i = 0; i < renderPassCreateInfo.colorAttachments.size(); i++)
        {
            const auto& renderPassColorAttachment = renderPassCreateInfo.colorAttachments[i];
            const auto& frameBufferColorAttachment = frameBufferCreateInfo.colorAttachments[i];

            if(renderPassColorAttachment.has_value())
            {
                auto mtlTexture = static_cast<MetalTexture*>(frameBufferColorAttachment.texture);
                LUCH_ASSERT(mtlTexture != nullptr);

                mtlpp::ClearColor clearColor = {
                    renderPassColorAttachment->clearValue.red,
                    renderPassColorAttachment->clearValue.green,
                    renderPassColorAttachment->clearValue.blue,
                    renderPassColorAttachment->clearValue.alpha
                };

                d.GetColorAttachments()[i].SetTexture(mtlTexture->GetNativeTexture());
                d.GetColorAttachments()[i].SetLevel(frameBufferColorAttachment.mipmapLevel);
                d.GetColorAttachments()[i].SetSlice(frameBufferColorAttachment.slice);
                d.GetColorAttachments()[i].SetDepthPlane(frameBufferColorAttachment.depthPlane);
                d.GetColorAttachments()[i].SetClearColor(clearColor);
                d.GetColorAttachments()[i].SetLoadAction(ToMetalLoadAction(renderPassColorAttachment->colorLoadOperation));
                d.GetColorAttachments()[i].SetStoreAction(ToMetalStoreAction(renderPassColorAttachment->colorStoreOperation));
            }
        }

        if(renderPassCreateInfo.depthStencilAttachment.has_value())
        {
            const auto& renderPassDepthStencilAttachment = renderPassCreateInfo.depthStencilAttachment;
            const auto& frameBufferDepthStencilAttachment = frameBufferCreateInfo.depthStencilAttachment;

            auto mtlDepthStencilBuffer = static_cast<MetalTexture*>(frameBufferDepthStencilAttachment.texture);
            LUCH_ASSERT(mtlDepthStencilBuffer != nullptr);

            if(FormatHasDepth(renderPassDepthStencilAttachment->format))
            {
                d.GetDepthAttachment().SetTexture(mtlDepthStencilBuffer->GetNativeTexture());
                d.GetDepthAttachment().SetLevel(frameBufferDepthStencilAttachment.mipmapLevel);
                d.GetDepthAttachment().SetSlice(frameBufferDepthStencilAttachment.slice);
                d.GetDepthAttachment().SetDepthPlane(frameBufferDepthStencilAttachment.depthPlane);
                d.GetDepthAttachment().SetClearDepth(renderPassDepthStencilAttachment->depthClearValue);
                d.GetDepthAttachment().SetLoadAction(ToMetalLoadAction(renderPassDepthStencilAttachment->depthLoadOperation));
                d.GetDepthAttachment().SetStoreAction(ToMetalStoreAction(renderPassDepthStencilAttachment->depthStoreOperation));
            }

            if(FormatHasStencil(renderPassDepthStencilAttachment->format))
            {
                d.GetStencilAttachment().SetTexture(mtlDepthStencilBuffer->GetNativeTexture());
                d.GetStencilAttachment().SetLevel(frameBufferDepthStencilAttachment.mipmapLevel);
                d.GetStencilAttachment().SetSlice(frameBufferDepthStencilAttachment.slice);
                d.GetStencilAttachment().SetDepthPlane(frameBufferDepthStencilAttachment.depthPlane);
                d.GetStencilAttachment().SetClearStencil(renderPassDepthStencilAttachment->stencilClearValue);
                d.GetStencilAttachment().SetLoadAction(ToMetalLoadAction(renderPassDepthStencilAttachment->stencilLoadOperation));
                d.GetStencilAttachment().SetStoreAction(ToMetalStoreAction(renderPassDepthStencilAttachment->stencilStoreOperation));
            }
        }

        return d;
    }
}
