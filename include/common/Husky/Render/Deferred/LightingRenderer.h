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
#include <Husky/Render/Deferred/LightingPassResources.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    class LightingRenderer
    {
    public:
        static constexpr int32 SharedUniformBufferSize = 16 * 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr int32 OffscreenImageCount = 3;
        static const String RendererName;

        LightingRenderer();
        ~LightingRenderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        const SharedPtr<DeferredResources>& GetDeferredResources() { return commonResources; }
        void SetDeferredResources(const SharedPtr<DeferredResources>& aResources) { commonResources = aResources; }

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        LightingTextures* DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera, GBufferTextures* gbuffer);
    private:
        void UpdateLights(const RefPtrUnorderedSet<SceneV1::Node>& lightNodes);

        RefPtr<PipelineState> CreateLightingPipelineState(LightingPassResources* lighting);
        ResultValue<bool, UniquePtr<GBufferPassResources>> PrepareGBufferPassResources();
        ResultValue<bool, UniquePtr<LightingPassResources>> PrepareLightingPassResources();
        ResultValue<bool, UniquePtr<LightingTextures>> CreateLightingTextures();

        SharedPtr<RenderContext> context;
        SharedPtr<DeferredResources> commonResources;
        UniquePtr<LightingPassResources> resources;
        UniquePtr<LightingTextures> lightingTextures;

        Format diffuseFormat = Format::R32G32B32A32Sfloat;
        Format specularFormat = Format::R32G32B32A32Sfloat;
        float32 minDepth = 0.0;
        float32 maxDepth = 1.0;
    };
}

