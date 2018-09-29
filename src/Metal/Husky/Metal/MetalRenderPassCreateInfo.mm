#include <Husky/Metal/MetalRenderPassCreateInfo.h>
#include <Husky/Graphics/RenderPassCreateInfo.h>
#include <Husky/Metal/MetalTexture.h>

namespace Husky::Metal
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
            HUSKY_ASSERT_MSG(false, "Unknown load operation");
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
            HUSKY_ASSERT_MSG(false, "Unknown store operation");
            return mtlpp::StoreAction::DontCare;
        }
    }

    mtlpp::RenderPassDescriptor ToMetalRenderPasDescriptor(const RenderPassCreateInfo& createInfo)
    {
        mtlpp::RenderPassDescriptor d;

        for(int32 i = 0; i < createInfo.colorAttachments.size(); i++)
        {
            const auto& colorAttachment = createInfo.colorAttachments[i];

            auto mtlTexture = static_cast<MetalTexture*>(colorAttachment->output.texture);
            HUSKY_ASSERT(mtlTexture != nullptr);

//            auto mtlResolveTexture = static_cast<MetalTexture*>(colorAttachment->resolve.texture);
//            if(colorAttachment->colorStoreOperation == AttachmentStoreOperation::Resolve)
//            {
//                HUSKY_ASSERT(mtlResolveTexture != nullptr);
//            }

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

//            if(mtlResolveTexture != nullptr)
//            {
//                d.GetColorAttachments()[i].SetResolveTexture(mtlResolveTexture->GetNativeTexture());
//            }
//
//            d.GetColorAttachments()[i].SetResolveLevel(colorAttachment->resolve.mipmapLevel);
//            d.GetColorAttachments()[i].SetResolveSlice(colorAttachment->resolve.slice);
//            d.GetColorAttachments()[i].SetResolveDepthPlane(colorAttachment->resolve.depthPlane);

            d.GetColorAttachments()[i].SetClearColor(clearColor);
            d.GetColorAttachments()[i].SetLoadAction(ToMetalLoadAction(colorAttachment->colorLoadOperation));
            d.GetColorAttachments()[i].SetStoreAction(ToMetalStoreAction(colorAttachment->colorStoreOperation));
        }

        if(createInfo.depthStencilAttachment != nullptr)
        {
            auto mtlDepthStencilBuffer = static_cast<MetalTexture*>(createInfo.depthStencilAttachment->output.texture);
            HUSKY_ASSERT(mtlDepthStencilBuffer != nullptr);

    //        auto mtlResolveDepthStencilBuffer = static_cast<MetalTexture*>(createInfo.depthStencilAttachment->resolve.texture);

            d.GetDepthAttachment().SetTexture(mtlDepthStencilBuffer->GetNativeTexture());
            d.GetDepthAttachment().SetLevel(createInfo.depthStencilAttachment->output.mipmapLevel);
            d.GetDepthAttachment().SetSlice(createInfo.depthStencilAttachment->output.slice);
            d.GetDepthAttachment().SetDepthPlane(createInfo.depthStencilAttachment->output.depthPlane);

    //        if(mtlResolveDepthStencilBuffer != nullptr)
    //        {
    //            d.GetDepthAttachment().SetResolveTexture(mtlResolveDepthStencilBuffer->GetNativeTexture());
    //        }
    //        d.GetDepthAttachment().SetResolveLevel(createInfo.depthStencilAttachment->resolve.mipmapLevel);
    //        d.GetDepthAttachment().SetResolveSlice(createInfo.depthStencilAttachment->resolve.slice);
    //        d.GetDepthAttachment().SetResolveDepthPlane(createInfo.depthStencilAttachment->resolve.depthPlane);
    //
            d.GetDepthAttachment().SetClearDepth(createInfo.depthStencilAttachment->depthClearValue);
            d.GetDepthAttachment().SetLoadAction(ToMetalLoadAction(createInfo.depthStencilAttachment->depthLoadOperation));
            d.GetDepthAttachment().SetStoreAction(ToMetalStoreAction(createInfo.depthStencilAttachment->depthStoreOperation));

            d.GetStencilAttachment().SetTexture(mtlDepthStencilBuffer->GetNativeTexture());
            d.GetStencilAttachment().SetLevel(createInfo.depthStencilAttachment->output.mipmapLevel);
            d.GetStencilAttachment().SetSlice(createInfo.depthStencilAttachment->output.slice);
            d.GetStencilAttachment().SetDepthPlane(createInfo.depthStencilAttachment->output.depthPlane);
    //        d.GetStencilAttachment().SetResolveTexture(mtlResolveDepthStencilBuffer->GetNativeTexture());
    //        d.GetStencilAttachment().SetResolveLevel(createInfo.depthStencilAttachment->resolve.mipmapLevel);
    //        d.GetStencilAttachment().SetResolveSlice(createInfo.depthStencilAttachment->resolve.slice);
    //        d.GetStencilAttachment().SetResolveDepthPlane(createInfo.depthStencilAttachment->resolve.depthPlane);
            d.GetStencilAttachment().SetClearStencil(createInfo.depthStencilAttachment->stencilClearValue);
            d.GetStencilAttachment().SetLoadAction(ToMetalLoadAction(createInfo.depthStencilAttachment->stencilLoadOperation));
            d.GetStencilAttachment().SetStoreAction(ToMetalStoreAction(createInfo.depthStencilAttachment->stencilStoreOperation));
        }

        return d;
    }
}
