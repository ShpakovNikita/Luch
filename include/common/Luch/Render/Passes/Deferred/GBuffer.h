#pragma once

#include <Luch/Render/Passes/Deferred/DeferredConstants.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Passes::Deferred
{
    using namespace Graph;

    template<uint32 Count>
    struct GBufferImpl
    {
        Array<RenderMutableResource, Count> color;
        RenderMutableResource depthStencil;
    };

    template<int32 Count>
    struct GBufferReadOnlyImpl
    {
        GBufferReadOnlyImpl() = default;
        GBufferReadOnlyImpl(const GBufferImpl<Count>& gbuffer)
            : depthStencil(gbuffer.depthStencil)
        {
            std::copy(gbuffer.color.begin(), gbuffer.color.end(), color.begin());
        }

        Array<RenderResource, Count> color;
        RenderResource depthStencil;
    };

    using GBuffer = GBufferImpl<DeferredConstants::GBufferColorAttachmentCount>;
    using GBufferReadOnly = GBufferReadOnlyImpl<DeferredConstants::GBufferColorAttachmentCount>;
}
