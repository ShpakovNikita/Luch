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
#include <Luch/Render/Deferred/GBufferRenderPass.h>

namespace Luch::Render
{
    using namespace Graphics;

    class SceneRenderer
    {
    public:
        SceneRenderer(SharedPtr<RenderContext> context);
        ~SceneRenderer();

        bool Initialize();
        bool Deinitialize();

        void BeginRender();
        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        void DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera);
        void EndRender();
    private:
        UniquePtr<Graph::RenderGraphBuilder> builder;
        UniquePtr<Deferred::GBufferRenderPass> gbufferPass;
        UniquePtr<Deferred::GBufferPassResources> gbufferPassResources;
        SharedPtr<RenderContext> context;
        RefPtr<CommandPool> commandPool;
    };
}
