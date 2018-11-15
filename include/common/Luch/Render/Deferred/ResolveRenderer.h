#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/Deferred/ResolvePassResources.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    class ResolveRenderer
    {
    public:
        static constexpr int32 SharedUniformBufferSize = 16 * 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr int32 OffscreenImageCount = 3;
        static const String RendererName;

        ResolveRenderer();
        ~ResolveRenderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        const SharedPtr<DeferredResources>& GetDeferredResources() { return commonResources; }
        void SetDeferredResources(const SharedPtr<DeferredResources>& aResources) { commonResources = aResources; }

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        Texture* Resolve(SceneV1::Scene* scene, SceneV1::Camera* camera, GBufferTextures* gbuffer);
    private:
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        RefPtr<PipelineState> CreateResolvePipelineState(ResolvePassResources* lighting);
        ResultValue<bool, UniquePtr<ResolvePassResources>> PrepareResolvePassResources();
        ResultValue<bool, RefPtr<Texture>> CreateResolveTexture();

        SharedPtr<RenderContext> context;
        SharedPtr<DeferredResources> commonResources;
        UniquePtr<ResolvePassResources> resources;
        RefPtr<Texture> resolveTexture;

        Format colorFormat = Format::R32G32B32A32Sfloat;
    };
}
