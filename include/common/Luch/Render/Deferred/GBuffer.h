#pragma once

#include <Luch/Render/Deferred/DeferredConstants.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Deferred
{
    using namespace Graph;

    template<int32 Count>
    struct GBufferImpl
    {
        Array<RenderMutableResource, Count> color;
        RenderMutableResource depthStencil;
    };

    template<int32 Count>
    struct GBufferReadOnlyImpl
    {
        GBufferReadOnly() = default;
        GBufferReadOnly(const GBuffer<Count>& gbuffer)
            : color(gbuffer.color.begin(), gbuffer.color.end())
            , depthStencil(gbuffer.depthStencil)
        {
        }

        Array<RenderResource, Count> color;
        RenderResource depthStencil;
    };

    using GBuffer = GBufferImpl<DeferredConstants::GBufferColorAttachmentCount>;
    using GBufferReadOnly = GBufferReadOnly<DeferredConstants::GBufferColorAttachmentCount>;
}
