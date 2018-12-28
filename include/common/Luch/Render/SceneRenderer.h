#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialManager.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>

namespace Luch::Render
{
    using namespace Graphics;

    class SceneRenderer
    {
        static const int32 DescriptorSetCount = 2048;
        static const int32 DescriptorCount = 8192;
        static const int32 SharedBufferSize = 1024 * 1024;
    public:
        SceneRenderer(RefPtr<SceneV1::Scene> scene);
        ~SceneRenderer();

        bool Initialize(SharedPtr<RenderContext> context);
        bool Deinitialize();

        void BeginRender();
        bool PrepareScene();
        void UpdateScene();
        void DrawScene(SceneV1::Camera* camera);
        void EndRender();
    private:
        static ResultValue<bool, UniquePtr<CameraResources>> PrepareCameraResources(GraphicsDevice* device);

        bool UploadSceneTextures();
        bool UploadSceneBuffers();

        UniquePtr<MaterialManager> materialManager;
        UniquePtr<Graph::RenderGraphBuilder> builder;

        UniquePtr<Deferred::GBufferRenderPass> gbufferPass;
        UniquePtr<Deferred::ResolveRenderPass> resolvePass;
        UniquePtr<Deferred::TonemapRenderPass> tonemapPass;

        UniquePtr<Deferred::GBufferPersistentContext> gbufferPersistentContext;
        UniquePtr<Deferred::ResolvePersistentContext> resolvePersistentContext;
        UniquePtr<Deferred::TonemapPersistentContext> tonemapPersistentContext;

        UniquePtr<Deferred::GBufferTransientContext> gbufferTransientContext;
        UniquePtr<Deferred::ResolveTransientContext> resolveTransientContext;
        UniquePtr<Deferred::TonemapTransientContext> tonemapTransientContext;

        UniquePtr<CameraResources> cameraResources;
        SharedPtr<RenderContext> context;
        RefPtr<CommandPool> commandPool;
        RefPtr<DescriptorPool> descriptorPool;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<SceneV1::Scene> scene;
    };
}
