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
#include <Luch/Render/MaterialManager.h>
#include <Luch/Render/Deferred/GBufferRenderPass.h>

namespace Luch::Render
{
    using namespace Graphics;

    class SceneRenderer
    {
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
        bool UploadSceneTextures();
        bool UploadSceneBuffers();

        UniquePtr<MaterialManager> materialManager;
        UniquePtr<Graph::RenderGraphBuilder> builder;
        UniquePtr<Deferred::GBufferRenderPass> gbufferPass;
        UniquePtr<Deferred::GBufferPassResources> gbufferPassResources;
        SharedPtr<RenderContext> context;
        RefPtr<CommandPool> commandPool;
        RefPtr<SceneV1::Scene> scene;
    };
}
