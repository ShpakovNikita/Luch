#pragma once

namespace Luch::Render::Deferred
{
    class GBufferRenderPass;
    class ResolveRenderPass;
    class TonemapRenderPass;

    struct GBufferPersistentContext;
    struct GBufferTransientContext;
    struct ResolvePersistentContext;
    struct ResolveTransientContext;
    struct TonemapPersistentContext;
    struct TonemapTransientContext;
}
