#include <Luch/Render/SceneRenderer.h>
#include <Luch/Render/TextureUploader.h>
#include <Luch/Render/ShaderDefines.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/AttributeSemantic.h>

#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>

#include <Luch/Render/Deferred/GBufferRenderPass.h>
#include <Luch/Render/Deferred/GBufferPassResources.h>
#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Deferred;
    using namespace Graph;

    SceneRenderer::SceneRenderer(
        SharedPtr<RenderContext> aContext)
        : context(std::move(aContext))
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    bool SceneRenderer::Initialize()
    {
        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        commandPool = std::move(createdCommandPool);

        auto [createGBufferPassResourcesResult, createdGBufferPassResources] = GBufferRenderPass::PrepareGBufferPassResources(context);
        if(!createGBufferPassResourcesResult)
        {
            return false;
        }

        gbufferPassResources = std::move(createdGBufferPassResources);
    }

    bool SceneRenderer::Deinitialize()
    {
        commandPool.Release();
        gbufferPassResources.reset();
        return true;
    }

    void SceneRenderer::BeginRender()
    {
        builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = builder->Initialize(context->device, commandPool);
        LUCH_ASSERT(builderInitialized);

        gbufferPass = MakeUnique<GBufferRenderPass>(
            swapchainInfo.width,
            swapchainInfo.height,
            context,
            &builder);

        gbufferPass->SetResources(gbufferPassResources.get());
    }

    void SceneRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        auto swapchainInfo = context->swapchain->GetInfo();

        gbufferPass->SetScene(scene);

        gbufferPass->PrepareScene(scene);
    }

    void SceneRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        gbufferPass->UpdateScene(scene);
    }

    void SceneRenderer::DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera)
    {
        gbufferPass->SetCamera(camera);

        auto [buildResult, renderGraph] = builder.Build();
        auto commandLists = renderGraph->Execute();
        for(auto& commandList : commandLists)
        {
            context->commandQueue->Submit(commandList);
        }
    }

    void SceneRenderer::EndRender()
    {
        builder.reset();
        gbufferPass.reset();
    }
}
