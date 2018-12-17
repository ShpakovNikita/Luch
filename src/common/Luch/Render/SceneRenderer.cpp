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

#include <Luch/Render/Graph/RenderGraph.h>

namespace Luch::Render
{
    using namespace Graphics;

    SceneRenderer::SceneRenderer(
        SharedPtr<RenderContext> aContext)
        : context(std::move(aContext))
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        LUCH_ASSERT(createCommandPoolResult == GraphicsResult::Success);

        commandPool = std::move(createdCommandPool);

        auto builderInitialized = builder.Initialize(context->device, commandPool);
        LUCH_ASSERT(builderInitialized);

        auto swapchainInfo = context->swapchain->GetInfo();

        gbufferPass = MakeUnique<Deferred::GBufferRenderPass>(
            swapchainInfo.width,
            swapchainInfo.height,
            context,
            &builder);

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
}
