#pragma once

namespace Luch::Render::Passes::Deferred
{
    class GBufferRenderPass;
    class ResolveRenderPass;
    class ResolveComputeRenderPass;
    class TonemapRenderPass;

    struct GBufferPersistentContext;
    struct GBufferTransientContext;
    struct ResolvePersistentContext;
    struct ResolveTransientContext;
    struct ResolveComputePersistentContext;
    struct ResolveComputeTransientContext;
    struct TonemapPersistentContext;
    struct TonemapTransientContext;
}
