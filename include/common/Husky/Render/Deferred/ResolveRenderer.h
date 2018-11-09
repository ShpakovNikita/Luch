#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/ResultValue.h>
#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/Attachment.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/Common.h>
#include <Husky/Render/ShaderDefines.h>
#include <Husky/Render/RenderForwards.h>
#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Render/Deferred/ResolvePassResources.h>

namespace Husky::Render::Deferred
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
