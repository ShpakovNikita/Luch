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
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/DescriptorPool.h>

#include <Luch/Render/RenderUtils.h>
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
        RefPtr<SceneV1::Scene> aScene)
        : scene(std::move(aScene))
    {
    }

    SceneRenderer::~SceneRenderer() = default;

    bool SceneRenderer::Initialize(SharedPtr<RenderContext> aContext)
    {
        context = aContext;
        materialManager = MakeUnique<MaterialManager>();

        auto materialManagerInitialized = materialManager->Initialize(context);

        if(!materialManagerInitialized)
        {
            return false;
        }

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        commandPool = std::move(createdCommandPool);

        auto [createGBufferPassResourcesResult, createdGBufferPassResources] = GBufferRenderPass::PrepareGBufferPassResources(
            context.get(),
            materialManager->GetResources());

        if(!createGBufferPassResourcesResult)
        {
            return false;
        }

        gbufferPassResources = std::move(createdGBufferPassResources);

        return true;
    }

    bool SceneRenderer::Deinitialize()
    {
        commandPool.Release();
        gbufferPassResources.reset();
        context.reset();

        auto materialManagerDeinitialized = materialManager->Deinitialize();
        if(!materialManagerDeinitialized)
        {
            return false;
        }

        return true;
    }

    void SceneRenderer::BeginRender()
    {
        builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = builder->Initialize(context->device, commandPool);
        LUCH_ASSERT(builderInitialized);

        auto swapchainInfo = context->swapchain->GetInfo();

        gbufferPass = MakeUnique<GBufferRenderPass>(
            swapchainInfo.width,
            swapchainInfo.height,
            context,
            builder.get());

        gbufferPass->SetGBufferPassResources(gbufferPassResources.get());
    }

    bool SceneRenderer::PrepareScene()
    {
        auto texturesUploaded = UploadSceneTextures();
        if(!texturesUploaded)
        {
            return false;
        }

        auto buffersUploaded = UploadSceneBuffers();
        if(!buffersUploaded)
        {
            return false;
        }

        auto& materials = scene->GetSceneProperties().materials;

        for(auto& material : materials)
        {
            auto materialPrepared = materialManager->PrepareMaterial(material);
            if(!materialPrepared)
            {
                return false;
            }
        }

        gbufferPass->SetScene(scene);

        gbufferPass->PrepareScene(scene);

        return true;
    }

    void SceneRenderer::UpdateScene()
    {
        auto& materials = scene->GetSceneProperties().materials;

        for(auto& material : materials)
        {
            materialManager->UpdateMaterial(material);
        }

        gbufferPass->UpdateScene(scene);
    }

    void SceneRenderer::DrawScene(SceneV1::Camera* camera)
    {
        gbufferPass->SetCamera(camera);

        auto [buildResult, renderGraph] = builder->Build();
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

    bool SceneRenderer::UploadSceneTextures()
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        const auto& textures = sceneProperties.textures;

        auto [createCommandPoolResult, commandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            texturesVector.push_back(texture);
        }

        TextureUploader textureUploader{ context->device, commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);

        if(!uploadTexturesSucceeded)
        {
            return false;
        }

        for (const auto& commandList : uploadTexturesResult.commandLists)
        {
            context->commandQueue->Submit(commandList);
        }

        return true;
    }

    bool SceneRenderer::UploadSceneBuffers()
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        const auto& buffers = sceneProperties.buffers;

        for(const auto& buffer : buffers)
        {
            [[maybe_unused]] bool uploadSucceeded = buffer->UploadToDevice(context->device);
            if(!uploadSucceeded)
            {
                return false;
            }
        }

        return true;
    }
}
