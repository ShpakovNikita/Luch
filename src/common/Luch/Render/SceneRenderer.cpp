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

#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayout.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>

#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Deferred/GBufferRenderPass.h>
#include <Luch/Render/Deferred/GBufferContext.h>
#include <Luch/Render/Deferred/ResolveRenderPass.h>
#include <Luch/Render/Deferred/ResolveContext.h>
#include <Luch/Render/Deferred/TonemapRenderPass.h>
#include <Luch/Render/Deferred/TonemapContext.h>
#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphResourcePool.h>

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

        auto [createCameraResourcesResult, createdCameraResources] = PrepareCameraResources(context->device);
        if(!createCameraResourcesResult)
        {
            return false;
        }

        cameraResources = std::move(createdCameraResources);

        materialManager = MakeUnique<MaterialManager>();

        auto materialManagerInitialized = materialManager->Initialize(context->device);

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

        auto [createGBufferPersistentContextResult, createdGBufferPersistentContext] = GBufferRenderPass::PrepareGBufferPersistentContext(
            context->device,
            cameraResources.get(),
            materialManager->GetResources());

        if(!createGBufferPersistentContextResult)
        {
            return false;
        }

        gbufferPersistentContext = std::move(createdGBufferPersistentContext);

        auto [createResolvePersistentContextResult, createdResolvePersistentContext] = ResolveRenderPass::PrepareResolvePersistentContext(
            context->device,
            cameraResources.get());

        if(!createResolvePersistentContextResult)
        {
            return false;
        }

        resolvePersistentContext = std::move(createdResolvePersistentContext);

        auto [createTonemapPersistentContextResult, createdTonemapPersistentContext] = TonemapRenderPass::PrepareTonemapPersistentContext(
            context->device,
            context->swapchain->GetInfo().format);

        if(!createTonemapPersistentContextResult)
        {
            return false;
        }

        tonemapPersistentContext = std::move(createdTonemapPersistentContext);

        BufferCreateInfo sharedBufferCreateInfo;
        sharedBufferCreateInfo.length = SharedBufferSize;
        sharedBufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        sharedBufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(sharedBufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            return false;
        }

        sharedBuffer = MakeShared<SharedBuffer>(std::move(createdBuffer));

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = DescriptorSetCount;
        descriptorPoolCreateInfo.descriptorCount = 
        {
            { ResourceType::Texture, DescriptorCount },
            { ResourceType::Sampler, DescriptorCount },
            { ResourceType::UniformBuffer, DescriptorCount },
        };

        auto [createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if(createDescriptorPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        descriptorPool = std::move(createdDescriptorPool);

        resourcePool = MakeUnique<RenderGraphResourcePool>(context->device);

        return true;
    }

    bool SceneRenderer::Deinitialize()
    {
        commandPool.Release();
        tonemapPersistentContext.reset();
        resolvePersistentContext.reset();
        gbufferPersistentContext.reset();
        context.reset();

        auto materialManagerDeinitialized = materialManager->Deinitialize();
        if(!materialManagerDeinitialized)
        {
            return false;
        }

        return true;
    }

    bool SceneRenderer::BeginRender()
    {
        auto [allocateCamreraDescriptorSetResult, allocatedCameraDescriptorSet] = descriptorPool->AllocateDescriptorSet(cameraResources->cameraBufferDescriptorSetLayout);
        if(allocateCamreraDescriptorSetResult != GraphicsResult::Success)
        {
            return false;
        }

        cameraDescriptorSet = std::move(allocatedCameraDescriptorSet);

        builder = MakeUnique<RenderGraphBuilder>();
        auto builderInitialized = builder->Initialize(context->device, commandPool, resourcePool.get());
        if(!builderInitialized)
        {
            return false;
        }

        auto swapchainInfo = context->swapchain->GetInfo();
        Size2i attachmentSize = { swapchainInfo.width, swapchainInfo.height };

        outputHandle = builder->GetResourceManager()->ImportAttachmentDeferred();

        auto [prepareGBufferTransientContextResult, preparedGBufferTransientContext] = GBufferRenderPass::PrepareGBufferTransientContext(
            gbufferPersistentContext.get(),
            descriptorPool);

        if(!prepareGBufferTransientContextResult)
        {
            return false;
        }

        gbufferTransientContext = std::move(preparedGBufferTransientContext);

        gbufferTransientContext->descriptorPool = descriptorPool;
        gbufferTransientContext->attachmentSize = attachmentSize;
        gbufferTransientContext->scene = scene;
        gbufferTransientContext->sharedBuffer = sharedBuffer;
        gbufferTransientContext->cameraBufferDescriptorSet = cameraDescriptorSet;

        gbufferPass = MakeUnique<GBufferRenderPass>(
            gbufferPersistentContext.get(),
            gbufferTransientContext.get(),
            builder.get());

        auto [prepareResolveTransientContextResult, preparedResolveTransientContext] = ResolveRenderPass::PrepareResolveTransientContext(
            resolvePersistentContext.get(),
            descriptorPool);

        if(!prepareResolveTransientContextResult)
        {
            return false;
        }

        resolveTransientContext = std::move(preparedResolveTransientContext);

        resolveTransientContext->gbuffer = gbufferPass->GetGBuffer();
        resolveTransientContext->attachmentSize = attachmentSize;
        resolveTransientContext->scene = scene;
        resolveTransientContext->sharedBuffer = sharedBuffer;
        resolveTransientContext->cameraBufferDescriptorSet = cameraDescriptorSet;

        resolvePass = MakeUnique<ResolveRenderPass>(
            resolvePersistentContext.get(),
            resolveTransientContext.get(),
            builder.get());

        auto [prepareTonemapTransientContextResult, preparedTonemapTransientContext] = TonemapRenderPass::PrepareTonemapTransientContext(
            tonemapPersistentContext.get(),
            descriptorPool);

        if(!prepareTonemapTransientContextResult)
        {
            return false;
        }

        tonemapTransientContext = std::move(preparedTonemapTransientContext);

        tonemapTransientContext->inputHandle = resolvePass->GetResolveTextureHandle();
        tonemapTransientContext->outputHandle = outputHandle;
        tonemapTransientContext->attachmentSize = attachmentSize;
        tonemapTransientContext->scene = scene;

        tonemapPass = MakeUnique<TonemapRenderPass>(
            tonemapPersistentContext.get(),
            tonemapTransientContext.get(),
            builder.get());

        return true;
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

        gbufferPass->PrepareScene();
        resolvePass->PrepareScene();
        tonemapPass->PrepareScene();

        return true;
    }

    void SceneRenderer::UpdateScene()
    {
        auto& materials = scene->GetSceneProperties().materials;

        for(auto& material : materials)
        {
            materialManager->UpdateMaterial(material, sharedBuffer.get());
        }

        gbufferPass->UpdateScene();
        resolvePass->UpdateScene();
        tonemapPass->UpdateScene();
    }

    void SceneRenderer::DrawScene(SceneV1::Node* cameraNode)
    {
        auto camera = cameraNode->GetCamera();
        LUCH_ASSERT(camera != nullptr);

        CameraUniform cameraUniform = RenderUtils::GetCameraUniform(camera, cameraNode->GetWorldTransform());
        auto suballocation = sharedBuffer->Suballocate(sizeof(CameraUniform), 256);

        memcpy(suballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));

        cameraDescriptorSet->WriteUniformBuffer(
            cameraResources->cameraUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        cameraDescriptorSet->Update();

        auto [getNextTextureResult, swapchainTexture] = context->swapchain->GetNextAvailableTexture(nullptr);
        LUCH_ASSERT(getNextTextureResult == GraphicsResult::Success);

        builder->GetResourceManager()->ProvideDeferredAttachment(outputHandle, swapchainTexture.texture);

        auto [buildResult, renderGraph] = builder->Build();
        auto commandLists = renderGraph->Execute();
        for(auto& commandList : commandLists)
        {
            context->commandQueue->Submit(commandList);
        }
    }

    void SceneRenderer::EndRender()
    {
        resourcePool->Tick();

        builder.reset();
        gbufferTransientContext.reset();
        resolveTransientContext.reset();
        tonemapTransientContext.reset();
        gbufferPass.reset();
        resolvePass.reset();
        tonemapPass.reset();

        sharedBuffer->Reset();
    }

    ResultValue<bool, UniquePtr<CameraResources>> SceneRenderer::PrepareCameraResources(GraphicsDevice* device)
    {
        UniquePtr<CameraResources> cameraResources = MakeUnique<CameraResources>();

        cameraResources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo
            .AddBinding(&cameraResources->cameraUniformBufferBinding)
            .OfType(DescriptorSetType::Buffer);

        auto [createDescriptorSetLayoutResult, createdDescriptorSetLayout] = device->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);
        if(createDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            return { false };
        }

        cameraResources->cameraBufferDescriptorSetLayout = std::move(createdDescriptorSetLayout);

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 10;
        descriptorPoolCreateInfo.descriptorCount = 
        {
            { ResourceType::UniformBuffer, 10}
        };

        auto [createDescriptorPoolResult, createdDescriptorPool] = device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if(createDescriptorPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        cameraResources->descriptorPool = std::move(createdDescriptorPool);

        return { true, std::move(cameraResources) };
    }

    bool SceneRenderer::UploadSceneTextures()
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        SceneV1::SceneProperties::Usages<SceneV1::Texture> textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            if(texture->GetDeviceTexture() != nullptr && texture->GetDeviceSampler() != nullptr)
            {
                continue;
            }

            texturesVector.push_back(texture);
        }

        if(texturesVector.empty())
        {
            return true;
        }

        auto [createCommandPoolResult, commandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
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
        SceneV1::SceneProperties::Usages<SceneV1::Buffer> buffers = sceneProperties.buffers;
    
        if(buffers.empty())
        {
            return false;
        }

        for(const auto& buffer : buffers)
        {
            if(buffer->IsResidentOnDevice())
            {
                continue;
            }

            [[maybe_unused]] bool uploadSucceeded = buffer->UploadToDevice(context->device);
            if(!uploadSucceeded)
            {
                return false;
            }
        }

        return true;
    }
}
