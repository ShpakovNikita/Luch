#pragma once

namespace Luch::Render::Deferred
{
    namespace ShadowMapping
    {
        class ShadowRenderer;
    }

    class DeferredRenderer;
    class GBufferRenderer;
    class ResolveRenderer;
    class TonemapRenderer;

    struct DeferredResources;
    struct GBufferPassResources;
    struct GBufferTextures;
    struct ResolvePassResources;
    struct TonemapPassResources;
}
