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
#include <Luch/Render/Passes/Deferred/DeferredForwards.h>
#include <Luch/Render/Passes/TiledDeferred/TiledDeferredForwards.h>
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
        UniquePtr<DepthOnlyRenderPass> depthOnlyPass;
        UniquePtr<TiledDeferred::TiledDeferredRenderPass> tiledDeferredPass;
        UniquePtr<Deferred::GBufferRenderPass> gbufferPass;
        UniquePtr<Deferred::ResolveRenderPass> resolvePass;
        UniquePtr<Deferred::ResolveComputeRenderPass> resolveComputePass;
        UniquePtr<TonemapRenderPass> tonemapPass;

        UniquePtr<DepthOnlyTransientContext> depthOnlyTransientContext;
        UniquePtr<TiledDeferred::TiledDeferredTransientContext> tiledDeferredTransientContext;
        UniquePtr<Deferred::GBufferTransientContext> gbufferTransientContext;
        UniquePtr<Deferred::ResolveTransientContext> resolveTransientContext;
        UniquePtr<Deferred::ResolveComputeTransientContext> resolveComputeTransientContext;
        UniquePtr<TonemapTransientContext> tonemapTransientContext;

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

        bool Initialize(SharedPtr<RenderContext> context, SceneRendererConfig config);
        bool Deinitialize();

        bool BeginRender();
        bool PrepareScene();
        void UpdateScene();
        void DrawScene(SceneV1::Node* cameraNode);
        void EndRender();
    private:
        bool PrepareDeferred(FrameResources& frame);
        bool PrepareTiledDeferred(FrameResources& frame);

        int32 GetCurrentFrameResourceIndex() const;
        static ResultValue<bool, UniquePtr<CameraResources>> PrepareCameraResources(GraphicsDevice* device);

        bool UploadSceneTextures();
        bool UploadSceneBuffers();

        bool canUseTiledDeferredRender = false;

        SceneRendererConfig config;

        UniquePtr<MaterialManager> materialManager;
        UniquePtr<Graph::RenderGraphResourcePool> resourcePool;

        UniquePtr<DepthOnlyPersistentContext> depthOnlyPersistentContext;
        UniquePtr<TiledDeferred::TiledDeferredPersistentContext> tiledDeferredPersistentContext;
        UniquePtr<Deferred::GBufferPersistentContext> gbufferPersistentContext;
        UniquePtr<Deferred::ResolvePersistentContext> resolvePersistentContext;
        UniquePtr<Deferred::ResolveComputePersistentContext> resolveComputePersistentContext;
        UniquePtr<TonemapPersistentContext> tonemapPersistentContext;

        Array<FrameResources, MaxSwapchainTextures> frameResources;
        UniquePtr<CameraResources> cameraResources;
        SharedPtr<RenderContext> context;
        RefPtr<Semaphore> semaphore;
        RefPtr<CommandPool> commandPool;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<SceneV1::Scene> scene;

        int32 frameIndex = 0;
    };
}
