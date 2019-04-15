#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/Passes/PassesForwards.h>
#include <Luch/Render/Passes/Forward/ForwardForwards.h>
#include <Luch/Render/Passes/Deferred/DeferredForwards.h>
#include <Luch/Render/Passes/TiledDeferred/TiledDeferredForwards.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialManager.h>
#include <Luch/Render/SceneRendererConfig.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Passes;

    struct FrameResources
    {
        Size2i outputSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorSet> cameraDescriptorSet;

        UniquePtr<Graph::RenderGraphBuilder> builder;
        UniquePtr<Graph::RenderGraph> renderGraph;

        UniquePtr<DepthOnlyRenderPass> depthOnlyPass;
        UniquePtr<Forward::ForwardRenderPass> forwardPass;
        UniquePtr<TiledDeferred::TiledDeferredRenderPass> tiledDeferredPass;
        UniquePtr<Deferred::GBufferRenderPass> gbufferPass;
        UniquePtr<Deferred::ResolveRenderPass> resolvePass;
        UniquePtr<Deferred::ResolveComputeRenderPass> resolveComputePass;
        UniquePtr<TonemapRenderPass> tonemapPass;

        UniquePtr<DepthOnlyTransientContext> depthOnlyTransientContext;
        UniquePtr<Forward::ForwardTransientContext> forwardTransientContext;
        UniquePtr<TiledDeferred::TiledDeferredTransientContext> tiledDeferredTransientContext;
        UniquePtr<Deferred::GBufferTransientContext> gbufferTransientContext;
        UniquePtr<Deferred::ResolveTransientContext> resolveTransientContext;
        UniquePtr<Deferred::ResolveComputeTransientContext> resolveComputeTransientContext;
        UniquePtr<TonemapTransientContext> tonemapTransientContext;

        Graph::RenderResource diffuseIlluminanceCubemapHandle;
        Graph::RenderResource specularReflectionCubemapHandle;
        Graph::RenderResource specularBRDFTextureHandle;

        Graph::RenderMutableResource outputHandle;
        RefPtr<SwapchainTexture> swapchainTexture;

        void Reset();
    };

    class SceneRenderer
    {
        static constexpr int32 MaxSwapchainTextures = 3;
        static constexpr int32 DescriptorSetCount = 2048;
        static constexpr int32 DescriptorCount = 8192;
        static constexpr int32 SharedBufferSize = 1024 * 1024;
    public:
        SceneRenderer(RefPtr<SceneV1::Scene> scene);
        ~SceneRenderer();

        bool Initialize(SharedPtr<RenderContext> context);
        bool Deinitialize();

        bool ProbeIndirectLighting();
        void ResetIndirectLighting();

        bool PrepareSceneResources();

        bool BeginRender();
        bool PrepareScene();
        void UpdateScene();
        void DrawScene(SceneV1::Node* cameraNode);
        void EndRender();

        inline SceneRendererConfig& GetMutableConfig() { return config; }
    private:
        bool PrepareForward(FrameResources& frame);
        bool PrepareDeferred(FrameResources& frame);
        bool PrepareTiledDeferred(FrameResources& frame);

        int32 GetCurrentFrameResourceIndex() const;
        UniquePtr<CameraPersistentResources> PrepareCameraResources(GraphicsDevice* device);
        UniquePtr<IndirectLightingPersistentResources> PrepareIndirectLightingResources(GraphicsDevice* device);
        UniquePtr<LightPersistentResources> PrepareLightResources(GraphicsDevice* device);

        bool UploadSceneTextures();
        bool UploadSceneBuffers();

        bool canUseTiledDeferredRender = false;

        SceneRendererConfig config;

        UniquePtr<IBLRenderer> iblRenderer;

        RefPtr<Texture> diffuseIlluminanceCubemap;
        RefPtr<Texture> specularReflectionCubemap;
        RefPtr<Texture> specularBRDFTexture;

        SharedPtr<MaterialManager> materialManager;
        UniquePtr<Graph::RenderGraphResourcePool> resourcePool;

        UniquePtr<DepthOnlyPersistentContext> depthOnlyPersistentContext;
        UniquePtr<Forward::ForwardPersistentContext> forwardPersistentContext;
        UniquePtr<TiledDeferred::TiledDeferredPersistentContext> tiledDeferredPersistentContext;
        UniquePtr<Deferred::GBufferPersistentContext> gbufferPersistentContext;
        UniquePtr<Deferred::ResolvePersistentContext> resolvePersistentContext;
        UniquePtr<Deferred::ResolveComputePersistentContext> resolveComputePersistentContext;
        UniquePtr<TonemapPersistentContext> tonemapPersistentContext;

        Array<FrameResources, MaxSwapchainTextures> frameResources;
        SharedPtr<CameraPersistentResources> cameraResources;
        SharedPtr<IndirectLightingPersistentResources> indirectLightingResources;
        SharedPtr<LightPersistentResources> lightResources;
        SharedPtr<RenderContext> context;
        RefPtr<Semaphore> semaphore;
        RefPtr<CommandPool> commandPool;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<SceneV1::Scene> scene;

        int32 frameIndex = 0;
    };
}
