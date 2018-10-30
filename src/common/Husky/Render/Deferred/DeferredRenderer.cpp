#include <Husky/Render/Deferred/DeferredRenderer.h>
#include <Husky/Render/TextureUploader.h>
#include <Husky/Render/ShaderDefines.h>

#include <Husky/Graphics/PrimitiveTopology.h>

#include <Husky/FileStream.h>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/AlphaMode.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Light.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/AttributeSemantic.h>

#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Graphics/TextureCreateInfo.h>
#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/ShaderLibrary.h>
#include <Husky/Graphics/DescriptorSet.h>
#include <Husky/Graphics/PhysicalDevice.h>
#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/Graphics/CommandQueue.h>
#include <Husky/Graphics/CommandPool.h>
#include <Husky/Graphics/DescriptorPool.h>
#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Graphics/Swapchain.h>
#include <Husky/Graphics/SwapchainInfo.h>
#include <Husky/Graphics/PipelineState.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/RenderPassCreateInfo.h>
#include <Husky/Graphics/DescriptorPoolCreateInfo.h>
#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Graphics/PipelineLayoutCreateInfo.h>
#include <Husky/Graphics/IndexType.h>
#include <Husky/Graphics/PipelineStateCreateInfo.h>

#include <Husky/Render/RenderUtils.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;
    using namespace ShadowMapping;

    // Fullscreen quad for triangle list
//    static const Vector<QuadVertex> fullscreenQuadVertices =
//    {
//        {{-1.0f, +1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom left
//        {{+1.0f, +1.0f, 0.0f}, {1.0f, 1.0f}}, // bottom right
//        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // top left
//
//        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},  // top left
//        {{+1.0f, +1.0f, 0.0f}, {1.0f, 1.0f}},  // bottom right
//        {{+1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // top right
//    };

    // One triangle that covers whole screen
    constexpr Array<QuadVertex, 3> fullscreenQuadVertices =
    {
        QuadVertex { Vec3{-1.0f, -1.0f, 0.0f}, Vec2{0.0f, 1.0f} },
        QuadVertex { Vec3{+3.0f, -1.0f, 0.0f}, Vec2{2.0f, 1.0f} },
        QuadVertex { Vec3{-1.0f, +3.0f, 0.0f}, Vec2{0.0f, -1.0f} },
    };

    // Think about passing these numbers through shader defines
    static const UnorderedMap<SceneV1::AttributeSemantic, int32> SemanticToLocation =
    {
        { SceneV1::AttributeSemantic::Position, 0 },
        { SceneV1::AttributeSemantic::Normal, 1 },
        { SceneV1::AttributeSemantic::Tangent, 2 },
        { SceneV1::AttributeSemantic::Texcoord_0, 3 },
        { SceneV1::AttributeSemantic::Texcoord_1, 4 },
        { SceneV1::AttributeSemantic::Color_0, 5 },
    };

    static const UnorderedMap<SceneV1::AttributeSemantic, DeferredShaderDefines> SemanticToFlag =
    {
        { SceneV1::AttributeSemantic::Position, DeferredShaderDefines::Empty },
        { SceneV1::AttributeSemantic::Normal, DeferredShaderDefines::HasNormal},
        { SceneV1::AttributeSemantic::Tangent, DeferredShaderDefines::HasTangent },
        { SceneV1::AttributeSemantic::Texcoord_0, DeferredShaderDefines::HasTexCoord0 },
    };

    static UnorderedMap<DeferredShaderDefines, String> FlagToString =
    {
        //{ ShaderDefine::HasPosition, "HAS_POSITION" },
        { DeferredShaderDefines::HasNormal, "HAS_NORMAL" },
        { DeferredShaderDefines::HasTangent, "HAS_TANGENT" },
        { DeferredShaderDefines::HasTexCoord0, "HAS_TEXCOORD_0" },
        { DeferredShaderDefines::HasBaseColorTexture, "HAS_BASE_COLOR_TEXTURE" },
        { DeferredShaderDefines::HasMetallicRoughnessTexture, "HAS_METALLIC_ROUGHNESS_TEXTURE" },
        { DeferredShaderDefines::HasNormalTexture, "HAS_NORMAL_TEXTURE" },
        { DeferredShaderDefines::HasOcclusionTexture, "HAS_OCCLUSION_TEXTURE" },
        { DeferredShaderDefines::HasEmissiveTexture, "HAS_EMISSIVE_TEXTURE" },
        { DeferredShaderDefines::AlphaMask, "ALPHA_MASK" },
    };

    const String DeferredRenderer::RendererName{"Deferred"};

    DeferredRenderer::DeferredRenderer(
        const RefPtr<PhysicalDevice>& physicalDevice,
        const RefPtr<Surface>& surface,
        int32 aWidth,
        int32 aHeight)
        : width(aWidth)
        , height(aHeight)
    {
        context = MakeShared<RenderContext>();
        context->physicalDevice = physicalDevice;
        context->surface = surface;
    }

    bool DeferredRenderer::Initialize()
    {
        auto[createDeviceResult, createdDevice] = context->physicalDevice->CreateGraphicsDevice();
        if (createDeviceResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->device = std::move(createdDevice);

        auto& device = context->device;

        auto[createCommandQueueResult, createdCommandQueue] = device->CreateCommandQueue();
        if (createCommandQueueResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->commandQueue = std::move(createdCommandQueue);

        SwapchainInfo swapchainCreateInfo;
        swapchainCreateInfo.format = swapchainFormat;
        swapchainCreateInfo.imageCount = 1;
        swapchainCreateInfo.width = width;
        swapchainCreateInfo.height = height;

        auto[createSwapchainResult, createdSwapchain] = device->CreateSwapchain(swapchainCreateInfo, context->surface);
        if (createSwapchainResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->swapchain = std::move(createdSwapchain);

        Vector<Format> depthFormats =
        {
            Format::D32SfloatS8Uint,
            Format::D24UnormS8Uint,
            Format::D16UnormS8Uint,
        };

        auto supportedDepthFormats = context->physicalDevice->GetSupportedDepthStencilFormats(depthFormats);
        HUSKY_ASSERT_MSG(!depthFormats.empty(), "No supported depth formats");
        depthStencilFormat = depthFormats.front();

        shadowRenderer = MakeUnique<ShadowRenderer>(context);
        bool shadowRendererInitialized = shadowRenderer->Initialize();
        if(!shadowRendererInitialized)
        {
            return false;
        }

        auto [prepareResourcesResult, preparedResources] = PrepareResources();
        if(prepareResourcesResult)
        {
            resources = std::move(preparedResources);
        }
        else
        {
            return false;
        }

        auto [gBufferResourcesPrepared, preparedGBufferResources] = PrepareGBufferPassResources();
        if (gBufferResourcesPrepared)
        {
            gbuffer = std::move(preparedGBufferResources);
        }
        else
        {
            return  false;
        }

        auto[lightingResourcesPrepared, preparedLightingResources] = PrepareLightingPassResources(gbuffer.get());
        if (lightingResourcesPrepared)
        {
            lighting = std::move(preparedLightingResources);
        }
        else
        {
            return  false ;
        }

        return true;
    }

    bool DeferredRenderer::Deinitialize()
    {
        resources.reset();

        bool shadowRendererDeinitialized = shadowRenderer->Deinitialize();
        if(!shadowRendererDeinitialized)
        {
            return false;
        }

        return true;
    }

    void DeferredRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        const auto& sceneProperties = scene->GetSceneProperties();
        const auto& textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            texturesVector.push_back(texture);
        }

        TextureUploader textureUploader{ context->device, resources->commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);

        HUSKY_ASSERT(uploadTexturesSucceeded);

        for (const auto& commandList : uploadTexturesResult.commandLists)
        {
            context->commandQueue->Submit(commandList);
        }

        const auto& buffers = sceneProperties.buffers;
        for(const auto& buffer : buffers)
        {
            bool uploadSucceeded = buffer->UploadToDevice(context->device);
            HUSKY_ASSERT(uploadSucceeded);
        }

        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            if (node->GetCamera() != nullptr)
            {
                PrepareCameraNode(node);
            }
            else if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node);
            }
            else if (node->GetLight() != nullptr)
            {
                PrepareLightNode(node);
            }
        }

        for (const auto& material : sceneProperties.materials)
        {
            PrepareMaterial(material);
        }

        PrepareLights(scene);

        shadowRenderer->PrepareScene(scene);
    }

    ResultValue<bool, UniquePtr<DeferredRendererResources>> DeferredRenderer::PrepareResources()
    {
        auto resources = MakeUnique<DeferredRendererResources>();

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::Texture, MaxDescriptorCount },
            { ResourceType::Sampler, MaxDescriptorCount },
            { ResourceType::UniformBuffer, MaxDescriptorCount },
        };
        descriptorPoolCreateInfo.maxDescriptorSets = MaxDescriptorSetCount;

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
            descriptorPoolCreateInfo);

        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        resources->descriptorPool = std::move(createdDescriptorPool);

        resources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources->cameraUniformBufferBinding);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            cameraDescriptorSetLayoutCreateInfo);

        if (createCameraDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources->cameraBufferDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        auto[createdCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if (createdCommandPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        resources->commandPool = std::move(createdCommandPool);

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedUniformBufferSize;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;

        auto [createSharedBufferResult, createdSharedBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createSharedBufferResult != GraphicsResult::Success)
        {
            return { false };
        }

        resources->sharedBuffer = MakeUnique<SharedBuffer>(createdSharedBuffer);

        return { true, std::move(resources) };
    }

    void DeferredRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        Mat4x4 identity = glm::mat4(1.0f);
        for (const auto& node : scene->GetNodes())
        {
            UpdateNode(node, identity);
        }

        shadowRenderer->UpdateScene(scene);
    }

    void DeferredRenderer::DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera)
    {
        const auto& lightNodesMap = scene->GetSceneProperties().lightNodes;
        RefPtrVector<SceneV1::Node> lightNodes;
        std::copy(lightNodesMap.begin(), lightNodesMap.end(), std::back_inserter(lightNodes));
        auto shadowMaps = shadowRenderer->DrawShadows(scene, lightNodes);

        auto[acquireResult, acquiredTexture] = context->swapchain->GetNextAvailableTexture(nullptr);
        HUSKY_ASSERT(acquireResult == GraphicsResult::Success);

        auto [allocateGBufferCommandListResult, gBufferCmdList] = resources->commandPool->AllocateGraphicsCommandList();
        HUSKY_ASSERT(allocateGBufferCommandListResult == GraphicsResult::Success);

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport { 0, 0, (float32)framebufferWidth, (float32)framebufferHeight, 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        ColorAttachment baseColorAttachment = gbuffer->baseColorAttachmentTemplate;
        baseColorAttachment.output.texture = gbuffer->offscreen.baseColorTexture;

        ColorAttachment normalMapAttachment = gbuffer->normalMapAttachmentTemplate;
        normalMapAttachment.output.texture = gbuffer->offscreen.normalMapTexture;

        DepthStencilAttachment depthStencilAttachment = gbuffer->depthStencilAttachmentTemplate;
        depthStencilAttachment.output.texture = gbuffer->offscreen.depthStencilBuffer;

        RenderPassCreateInfo gBufferRenderPassCreateInfo;
        gBufferRenderPassCreateInfo
            .WithNColorAttachments(2)
            .AddColorAttachment(&baseColorAttachment)
            .AddColorAttachment(&normalMapAttachment)
            .WithDepthStencilAttachment(&depthStencilAttachment);

        auto[createGBufferRenderPassResult, createdGBufferRenderPass] = context->device->CreateRenderPass(
            gBufferRenderPassCreateInfo);

        HUSKY_ASSERT(createGBufferRenderPassResult == GraphicsResult::Success);

        gBufferCmdList->Begin();
        gBufferCmdList->BeginRenderPass(gBufferRenderPassCreateInfo);
        gBufferCmdList->SetViewports({viewport});
        gBufferCmdList->SetScissorRects({scissorRect});
        gBufferCmdList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            gbuffer->pipelineLayout,
            camera->GetDescriptorSet(RendererName));

        for (const auto& node : scene->GetNodes())
        {
            DrawNode(node, gBufferCmdList);
        }

        gBufferCmdList->EndRenderPass();
        gBufferCmdList->End();

        context->commandQueue->Submit(gBufferCmdList);

        auto [allocateLightingCommandListResult, lightingCmdList] = resources->commandPool->AllocateGraphicsCommandList();
        HUSKY_ASSERT(allocateLightingCommandListResult == GraphicsResult::Success);

        ColorAttachment colorAttachment = lighting->colorAttachmentTemplate;
        colorAttachment.output.texture = acquiredTexture.texture;

        RenderPassCreateInfo lightingRenderPassCreateInfo;
        lightingRenderPassCreateInfo
            .WithNColorAttachments(1)
            .AddColorAttachment(&colorAttachment);

        lightingCmdList->Begin();
        lightingCmdList->BeginRenderPass(lightingRenderPassCreateInfo);
        lightingCmdList->BindPipelineState(lighting->pipelineState);
        lightingCmdList->SetViewports({ viewport });
        lightingCmdList->SetScissorRects({ scissorRect });

        lightingCmdList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            lighting->pipelineLayout,
            lighting->gbufferTextureDescriptorSet);

        lightingCmdList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            lighting->pipelineLayout,
            lighting->gbufferSamplerDescriptorSet);

        lightingCmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            lighting->pipelineLayout,
            camera->GetDescriptorSet(RendererName));

        lightingCmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            lighting->pipelineLayout,
            lighting->lightsBufferDescriptorSet);

        lightingCmdList->BindVertexBuffers({ lighting->fullscreenQuadBuffer }, {0}, 0);
        lightingCmdList->Draw(0, fullscreenQuadVertices.size());
        lightingCmdList->EndRenderPass();
        lightingCmdList->End();

        context->commandQueue->Submit(lightingCmdList);
        context->commandQueue->Present(acquiredTexture.index, context->swapchain);
    }

    void DeferredRenderer::PrepareCameraNode(SceneV1::Node* node)
    {
        const auto& camera = node->GetCamera();

        auto[createVertexDescriptorSetResult, vertexDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->cameraBufferDescriptorSetLayout);
        HUSKY_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);

        camera->SetDescriptorSet(RendererName, vertexDescriptorSet);
    }

    void DeferredRenderer::PrepareMeshNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }

        // TODO sort out node hierarchy
        for (const auto& child : node->GetChildren())
        {
            PrepareMeshNode(child);
        }
    }

    void DeferredRenderer::PrepareLightNode(SceneV1::Node* node)
    {
        HUSKY_ASSERT_MSG(node->GetChildren().empty(), "Don't add children to light nodes");
    }

    void DeferredRenderer::PrepareNode(SceneV1::Node* node)
    {
    }

    void DeferredRenderer::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        RefPtr<PipelineState> pipelineState = primitive->GetPipelineState(RendererName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreateGBufferPipelineState(primitive);
            primitive->SetPipelineState(RendererName, pipelineState);
        }
    }

    void DeferredRenderer::PrepareMesh(SceneV1::Mesh* mesh)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            PreparePrimitive(primitive);
        }

        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
            gbuffer->meshBufferDescriptorSetLayout);

        HUSKY_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);

        mesh->SetBufferDescriptorSet(RendererName, allocatedDescriptorSet);
    }

    void DeferredRenderer::PrepareMaterial(SceneV1::PbrMaterial* material)
    {
        auto[allocateTextureDescriptorSetResult, textureDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
            gbuffer->materialTextureDescriptorSetLayout);
        HUSKY_ASSERT(allocateTextureDescriptorSetResult == GraphicsResult::Success);

        auto[allocateBufferDescriptorSetResult, bufferDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
            gbuffer->materialBufferDescriptorSetLayout);
        HUSKY_ASSERT(allocateBufferDescriptorSetResult == GraphicsResult::Success);

        auto[allocateSamplerDescriptorSetResult, samplerDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
            gbuffer->materialSamplerDescriptorSetLayout);
        HUSKY_ASSERT(allocateSamplerDescriptorSetResult == GraphicsResult::Success);

        material->SetTextureDescriptorSet(RendererName, textureDescriptorSet);
        material->SetBufferDescriptorSet(RendererName, bufferDescriptorSet);
        material->SetSamplerDescriptorSet(RendererName, samplerDescriptorSet);

        UpdateMaterial(material);
    }

    void DeferredRenderer::PrepareLights(SceneV1::Scene* scene)
    {
        constexpr int MAX_LIGHTS_COUNT = 8;
        int32 lightsBufferSize = sizeof(LightUniform) * MAX_LIGHTS_COUNT;

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = lightsBufferSize;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createLightsBufferResult, createdLightsBuffer] = context->device->CreateBuffer(bufferCreateInfo);

        HUSKY_ASSERT(createLightsBufferResult == GraphicsResult::Success);
        lighting->lightsBuffer = createdLightsBuffer;

        auto [mapMemoryResult, _] = lighting->lightsBuffer->MapMemory(lightsBufferSize, 0);
        HUSKY_ASSERT(mapMemoryResult == GraphicsResult::Success);

        auto [createDescriptorSetResult, createdDescriptorSet] = lighting->descriptorPool->AllocateDescriptorSet(
            lighting->lightsBufferDescriptorSetLayout);
        HUSKY_ASSERT(createDescriptorSetResult == GraphicsResult::Success);
        lighting->lightsBufferDescriptorSet = createdDescriptorSet;

        createdDescriptorSet->WriteUniformBuffer(lighting->lightsUniformBufferBinding, createdLightsBuffer, 0);
        createdDescriptorSet->Update();
    }

    void DeferredRenderer::UpdateNode(SceneV1::Node* node, const Mat4x4& parentTransform)
    {
        const auto& mesh = node->GetMesh();

        Mat4x4 localTransformMatrix;
        const auto& localTransform = node->GetLocalTransform();

        if (std::holds_alternative<Mat4x4>(localTransform))
        {
            localTransformMatrix = std::get<Mat4x4>(localTransform);
        }
        else if(std::holds_alternative<SceneV1::TransformProperties>(localTransform))
        {
            const auto& transformProperties = std::get<SceneV1::TransformProperties>(localTransform);

            localTransformMatrix
                = glm::translate(transformProperties.translation)
                * glm::toMat4(transformProperties.rotation)
                * glm::scale(transformProperties.scale);
        }

        Mat4x4 worldTransform = parentTransform * localTransformMatrix;
        node->SetWorldTransform(worldTransform);

        if (mesh != nullptr)
        {
            UpdateMesh(mesh, worldTransform);
        }

        const auto& camera = node->GetCamera();

        if (camera != nullptr)
        {
            UpdateCamera(camera, worldTransform);
        }

        const auto& light = node->GetLight();
        if (light != nullptr)
        {
            UpdateLight(light, worldTransform);
        }

        for (const auto& child : node->GetChildren())
        {
            UpdateNode(child, worldTransform);
        }
    }

    void DeferredRenderer::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
    {
        MeshUniform meshUniform;
        meshUniform.transform = transform;
        meshUniform.inverseTransform = glm::inverse(transform);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MeshUniform), 16);
        auto descriptorSet = mesh->GetBufferDescriptorSet(RendererName);

        descriptorSet->WriteUniformBuffer(
            gbuffer->meshUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);
        descriptorSet->Update();

        memcpy(suballocation.offsetMemory, &meshUniform, sizeof(MeshUniform));
    }

    void DeferredRenderer::UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform)
    {
        camera->SetCameraViewMatrix(glm::inverse(transform));
        auto cameraUniform = RenderUtils::GetCameraUniform(camera);
        auto descriptorSet = camera->GetDescriptorSet(RendererName);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(CameraUniform), 16);

        descriptorSet->WriteUniformBuffer(
            resources->cameraUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        descriptorSet->Update();

        memcpy(suballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));
    }

    void DeferredRenderer::UpdateMaterial(SceneV1::PbrMaterial* material)
    {
        auto textureDescriptorSet = material->GetTextureDescriptorSet(RendererName);
        auto samplerDescriptorSet = material->GetSamplerDescriptorSet(RendererName);
        auto bufferDescriptorSet = material->GetBufferDescriptorSet(RendererName);

        if (material->HasBaseColorTexture())
        {
            textureDescriptorSet->WriteTexture(
                gbuffer->baseColorTextureBinding,
                material->metallicRoughness.baseColorTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                gbuffer->baseColorSamplerBinding,
                material->metallicRoughness.baseColorTexture.texture->GetDeviceSampler());
        }

        if (material->HasMetallicRoughnessTexture())
        {
            textureDescriptorSet->WriteTexture(
                gbuffer->metallicRoughnessTextureBinding,
                material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                gbuffer->metallicRoughnessSamplerBinding,
                material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceSampler());
        }

        if (material->HasNormalTexture())
        {
            textureDescriptorSet->WriteTexture(
                gbuffer->normalTextureBinding,
                material->normalTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                gbuffer->normalSamplerBinding,
                material->normalTexture.texture->GetDeviceSampler());
        }

        if (material->HasOcclusionTexture())
        {
            textureDescriptorSet->WriteTexture(
                gbuffer->occlusionTextureBinding,
                material->occlusionTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                gbuffer->occlusionSamplerBinding,
                material->occlusionTexture.texture->GetDeviceSampler());
        }

        if (material->HasEmissiveTexture())
        {
            textureDescriptorSet->WriteTexture(
                gbuffer->emissiveTextureBinding,
                material->emissiveTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                gbuffer->emissiveSamplerBinding,
                material->emissiveTexture.texture->GetDeviceSampler());
        }

        MaterialUniform materialUniform = RenderUtils::GetMaterialUniform(material);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MaterialUniform), 16);

        bufferDescriptorSet->WriteUniformBuffer(
            gbuffer->materialUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        memcpy(suballocation.offsetMemory, &materialUniform, sizeof(MaterialUniform));

        textureDescriptorSet->Update();
        samplerDescriptorSet->Update();
        bufferDescriptorSet->Update();
    }

    void DeferredRenderer::UpdateLight(SceneV1::Light* light, const Mat4x4& transform)
    {
        LightUniform lightUniform = RenderUtils::GetLightUniform(light, transform);

        memcpy((LightUniform*)lighting->lightsBuffer->GetMappedMemory() + light->GetIndex(), &lightUniform, sizeof(LightUniform));
    }

    void DeferredRenderer::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
    {
        const auto& mesh = node->GetMesh();
        if (mesh != nullptr)
        {
            DrawMesh(mesh, commandList);
        }

        for (const auto& child : node->GetChildren())
        {
            DrawNode(child, commandList);
        }
    }

    void DeferredRenderer::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            gbuffer->pipelineLayout,
            mesh->GetBufferDescriptorSet(RendererName));

        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->alphaMode != SceneV1::AlphaMode::Blend)
            {
                BindMaterial(material, commandList);
                DrawPrimitive(primitive, commandList);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->alphaMode == SceneV1::AlphaMode::Blend)
            {
                BindMaterial(material, commandList);
                DrawPrimitive(primitive, commandList);
            }
        }
    }

    void DeferredRenderer::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
    {
        commandList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            gbuffer->pipelineLayout,
            material->GetTextureDescriptorSet(RendererName));

        commandList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            gbuffer->pipelineLayout,
            material->GetBufferDescriptorSet(RendererName));

        commandList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            gbuffer->pipelineLayout,
            material->GetSamplerDescriptorSet(RendererName));
    }

    void DeferredRenderer::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
    {
        auto& pipelineState = primitive->GetPipelineState(RendererName);

        const auto& vertexBuffers = primitive->GetVertexBuffers();

        Vector<Buffer*> graphicsVertexBuffers;
        Vector<int32> offsets;
        graphicsVertexBuffers.reserve(vertexBuffers.size());
        offsets.reserve(vertexBuffers.size());

        for (const auto& vertexBuffer : vertexBuffers)
        {
            graphicsVertexBuffers.push_back(vertexBuffer.backingBuffer->GetDeviceBuffer());
            offsets.push_back(vertexBuffer.byteOffset);
        }

        HUSKY_ASSERT(primitive->GetIndexBuffer().has_value());
        const auto& indexBuffer = *primitive->GetIndexBuffer();

        commandList->BindPipelineState(pipelineState);
        commandList->BindVertexBuffers(graphicsVertexBuffers, offsets, 0);

        commandList->BindIndexBuffer(
                indexBuffer.backingBuffer->GetDeviceBuffer(),
                indexBuffer.indexType,
                indexBuffer.byteOffset);

        commandList->DrawIndexedInstanced(indexBuffer.count, 0, 1, 0);
    }

    RefPtr<PipelineState> DeferredRenderer::CreateGBufferPipelineState(SceneV1::Primitive* primitive)
    {
        PipelineStateCreateInfo ci;

        ShaderDefines<DeferredShaderDefines> shaderDefines;
        shaderDefines.mapping = &FlagToString;

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        HUSKY_ASSERT(vertexBuffers.size() == 1);

        ci.inputAssembler.bindings.resize(vertexBuffers.size());
        for (int i = 0; i < vertexBuffers.size(); i++)
        {
            const auto& vertexBuffer = vertexBuffers[i];
            auto& bindingDescription = ci.inputAssembler.bindings[i];
            bindingDescription.stride = vertexBuffer.stride;
            bindingDescription.inputRate = VertexInputRate::PerVertex;
        }

        const auto& attributes = primitive->GetAttributes();
        ci.inputAssembler.attributes.resize(SemanticToLocation.size());
        for (const auto& attribute : attributes)
        {
            auto& attributeDescription = ci.inputAssembler.attributes[SemanticToLocation.at(attribute.semantic)];
            attributeDescription.binding = attribute.vertexBufferIndex;
            attributeDescription.format = attribute.format;
            attributeDescription.offset = attribute.offset;

            shaderDefines.AddFlag(SemanticToFlag.at(attribute.semantic));
        }

        // TODO
        ci.inputAssembler.primitiveTopology = PrimitiveTopology::TriangleList;

        const auto& material = primitive->GetMaterial();

        if (material->doubleSided)
        {
            ci.rasterization.cullMode = CullMode::None;
        }
        else
        {
            ci.rasterization.cullMode = CullMode::Back;
        }

        // TODO
        ci.rasterization.cullMode = CullMode::None;

        ci.depthStencil.depthTestEnable = true;
        ci.depthStencil.depthWriteEnable = true;
        ci.depthStencil.depthCompareFunction = CompareFunction::Less;

        auto& baseColorAttachment = ci.colorAttachments.attachments.emplace_back();

        baseColorAttachment.blendEnable = material->alphaMode == SceneV1::AlphaMode::Blend;
        baseColorAttachment.format = baseColorFormat;
        if (baseColorAttachment.blendEnable)
        {
            baseColorAttachment.srcColorBlendFactor = BlendFactor::SourceAlpha;
            baseColorAttachment.dstColorBlendFactor = BlendFactor::OneMinusSourceAlpha;
            baseColorAttachment.colorBlendOp = BlendOperation::Add;
            baseColorAttachment.srcAlphaBlendFactor = BlendFactor::OneMinusSourceAlpha;
            baseColorAttachment.dstAlphaBlendFactor = BlendFactor::Zero;
            baseColorAttachment.alphaBlendOp = BlendOperation::Add;
        }

        auto& normalMapAttachmentBlendState = ci.colorAttachments.attachments.emplace_back();
        normalMapAttachmentBlendState.format = normalMapFormat;
        normalMapAttachmentBlendState.blendEnable = false;

        // TODO
        //ci.renderPass = scene.gbuffer.renderPass;
        ci.pipelineLayout = gbuffer->pipelineLayout;

        if (material->HasBaseColorTexture())
        {
            shaderDefines.AddFlag(DeferredShaderDefines::HasBaseColorTexture);
        }

        if (material->HasMetallicRoughnessTexture())
        {
            shaderDefines.AddFlag(DeferredShaderDefines::HasMetallicRoughnessTexture);
        }

        if (material->HasNormalTexture())
        {
            shaderDefines.AddFlag(DeferredShaderDefines::HasNormalTexture);
        }

        if (material->HasOcclusionTexture())
        {
            shaderDefines.AddFlag(DeferredShaderDefines::HasOcclusionTexture);
        }

        if (material->HasEmissiveTexture())
        {
            shaderDefines.AddFlag(DeferredShaderDefines::HasEmissiveTexture);
        }

        if (material->alphaMode == SceneV1::AlphaMode::Mask)
        {
            shaderDefines.AddFlag(DeferredShaderDefines::AlphaMask);
        }

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/gbuffer_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/gbuffer.vert",
    #else
            "",
    #endif
#endif
            shaderDefines.defines);

        if (!vertexShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
        }

        auto[vertexShaderCreateResult, createdVertexShader] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "vp_main");
        HUSKY_ASSERT(vertexShaderCreateResult == GraphicsResult::Success);

        auto vertexShader = std::move(createdVertexShader);

        auto[fragmentShaderLibraryCreated, fragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/gbuffer_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/gbuffer.frag",
    #else
            "",
    #endif
#endif
            shaderDefines.defines);

        if (!fragmentShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
        }

        auto[fragmentShaderCreateResult, createdFragmentShader] = fragmentShaderLibrary->CreateShaderProgram(
            ShaderStage::Fragment,
            "fp_main");
        HUSKY_ASSERT(fragmentShaderCreateResult == GraphicsResult::Success);

        auto fragmentShader = std::move(createdFragmentShader);

        // Retain shader modules
        primitive->AddShaderProgram(vertexShader);
        primitive->AddShaderProgram(fragmentShader);

        ci.vertexProgram = vertexShader;
        ci.fragmentProgram = fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    RefPtr<PipelineState> DeferredRenderer::CreateLightingPipelineState(LightingPassResources* lighting)
    {
        PipelineStateCreateInfo ci;

        auto& bindingDescription = ci.inputAssembler.bindings.emplace_back();
        bindingDescription.stride = sizeof(QuadVertex);
        bindingDescription.inputRate = VertexInputRate::PerVertex;

        auto& positionAttributeDescription = ci.inputAssembler.attributes.emplace_back();
        positionAttributeDescription.binding = 0;
        positionAttributeDescription.format = Format::R32G32B32Sfloat;
        positionAttributeDescription.offset = offsetof(QuadVertex, position);

        auto& texCoordAttributeDescription = ci.inputAssembler.attributes.emplace_back();
        texCoordAttributeDescription.binding = 0;
        texCoordAttributeDescription.format = Format::R32G32Sfloat;
        texCoordAttributeDescription.offset = offsetof(QuadVertex, texCoord);

        ci.inputAssembler.primitiveTopology = PrimitiveTopology::TriangleList;
        ci.rasterization.cullMode = CullMode::Back;
        ci.rasterization.frontFace = FrontFace::CounterClockwise;

        ci.depthStencil.depthTestEnable = false;
        ci.depthStencil.depthWriteEnable = false;

        auto& outputAttachment = ci.colorAttachments.attachments.emplace_back();
        outputAttachment.format = context->swapchain->GetInfo().format;
        outputAttachment.blendEnable = false;

        ci.depthStencil.depthStencilFormat = Format::Undefined;

        //pipelineState.renderPass = lighting.renderPass;
        ci.pipelineLayout = lighting->pipelineLayout;
        ci.vertexProgram = lighting->vertexShader;
        ci.fragmentProgram = lighting->fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<GBufferPassResources>> DeferredRenderer::PrepareGBufferPassResources()
    {
        UniquePtr<GBufferPassResources> gbufferResources = MakeUnique<GBufferPassResources>();

        auto [offscreenTexturesCreated, createdOffscreenTextures] = CreateOffscreenTextures();
        if (!offscreenTexturesCreated)
        {
            return { false };
        }

        gbufferResources->offscreen = std::move(createdOffscreenTextures);

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = MaxDescriptorSetCount;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::Texture, MaxDescriptorSetCount },
            { ResourceType::UniformBuffer, MaxDescriptorSetCount },
            { ResourceType::Sampler, MaxDescriptorSetCount },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
            descriptorPoolCreateInfo);

        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        gbufferResources->descriptorPool = std::move(createdDescriptorPool);

        gbufferResources->materialUniformBufferBinding.OfType(ResourceType::UniformBuffer);
        gbufferResources->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        gbufferResources->baseColorTextureBinding.OfType(ResourceType::Texture);
        gbufferResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);

        gbufferResources->metallicRoughnessTextureBinding.OfType(ResourceType::Texture);
        gbufferResources->metallicRoughnessSamplerBinding.OfType(ResourceType::Sampler);

        gbufferResources->normalTextureBinding.OfType(ResourceType::Texture);
        gbufferResources->normalSamplerBinding.OfType(ResourceType::Sampler);

        gbufferResources->occlusionTextureBinding.OfType(ResourceType::Texture);
        gbufferResources->occlusionSamplerBinding.OfType(ResourceType::Sampler);

        gbufferResources->emissiveTextureBinding.OfType(ResourceType::Texture);
        gbufferResources->emissiveSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
        meshDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .AddBinding(&gbufferResources->meshUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialBufferDescriptorSetLayoutCreateInfo;
        materialBufferDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .AddBinding(&gbufferResources->materialUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialTextureDescriptorSetLayoutCreateInfo;
        materialTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(5)
            .AddBinding(&gbufferResources->baseColorTextureBinding)
            .AddBinding(&gbufferResources->metallicRoughnessTextureBinding)
            .AddBinding(&gbufferResources->normalTextureBinding)
            .AddBinding(&gbufferResources->occlusionTextureBinding)
            .AddBinding(&gbufferResources->emissiveTextureBinding);

        DescriptorSetLayoutCreateInfo materialSamplerDescriptorSetLayoutCreateInfo;
        materialSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(5)
            .AddBinding(&gbufferResources->baseColorSamplerBinding)
            .AddBinding(&gbufferResources->metallicRoughnessSamplerBinding)
            .AddBinding(&gbufferResources->normalSamplerBinding)
            .AddBinding(&gbufferResources->occlusionSamplerBinding)
            .AddBinding(&gbufferResources->emissiveSamplerBinding);

        auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            meshDescriptorSetLayoutCreateInfo);

        if (createMeshDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        gbufferResources->meshBufferDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);

        auto[createMaterialTextureDescriptorSetLayoutResult, createdMaterialTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            materialTextureDescriptorSetLayoutCreateInfo);

        if (createMaterialTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        gbufferResources->materialTextureDescriptorSetLayout = std::move(createdMaterialTextureDescriptorSetLayout);

        auto[createMaterialBufferDescriptorSetLayoutResult, createdMaterialBufferDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            materialBufferDescriptorSetLayoutCreateInfo);

        if (createMaterialBufferDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        gbufferResources->materialBufferDescriptorSetLayout = std::move(createdMaterialBufferDescriptorSetLayout);

        auto[createMaterialSamplerDescriptorSetLayoutResult, createdMaterialSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            materialSamplerDescriptorSetLayoutCreateInfo);

        if (createMaterialSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        gbufferResources->materialSamplerDescriptorSetLayout = std::move(createdMaterialSamplerDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Vertex, resources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Vertex, gbufferResources->meshBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(
            pipelineLayoutCreateInfo);

        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        gbufferResources->pipelineLayout = std::move(createdPipelineLayout);

        gbufferResources->baseColorAttachmentTemplate.format = baseColorFormat;
        gbufferResources->baseColorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        gbufferResources->baseColorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
        gbufferResources->baseColorAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

        gbufferResources->normalMapAttachmentTemplate.format = normalMapFormat;
        gbufferResources->normalMapAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        gbufferResources->normalMapAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
        gbufferResources->normalMapAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };

        gbufferResources->depthStencilAttachmentTemplate.format = depthStencilFormat;
        gbufferResources->depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
        gbufferResources->depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::Store;
        gbufferResources->depthStencilAttachmentTemplate.stencilLoadOperation = AttachmentLoadOperation::Clear;
        gbufferResources->depthStencilAttachmentTemplate.stencilStoreOperation = AttachmentStoreOperation::Store;
        gbufferResources->depthStencilAttachmentTemplate.depthClearValue = maxDepth;
        gbufferResources->depthStencilAttachmentTemplate.stencilClearValue = 0xffffffff;

        return { true, std::move(gbufferResources) };
    }

    ResultValue<bool, UniquePtr<LightingPassResources>> DeferredRenderer::PrepareLightingPassResources(
        GBufferPassResources* gbufferPassResources)
    {
        UniquePtr<LightingPassResources> lightingResources = MakeUnique<LightingPassResources>();
        int32 swapchainLength = context->swapchain->GetInfo().imageCount;

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 1 + swapchainLength + 1;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, 1 },
            { ResourceType::Texture, swapchainLength * OffscreenImageCount + 1 },
            { ResourceType::Sampler, swapchainLength * OffscreenImageCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
            descriptorPoolCreateInfo);
        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->descriptorPool = std::move(createdDescriptorPool);

        ShaderDefines<DeferredShaderDefines> shaderDefines;
        shaderDefines.mapping = &FlagToString;

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.vert",
    #else
            "",
    #endif
#endif
            shaderDefines.defines);

        if (!vertexShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        auto [vertexShaderProgramCreateResult, vertexShaderProgram] = createdVertexShaderLibrary->CreateShaderProgram(ShaderStage::Vertex, "vp_main");
        if(vertexShaderProgramCreateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.frag",
    #else
            "",
    #endif
#endif
            shaderDefines.defines);

        if (!fragmentShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        auto [fragmentShaderProgramCreateResult, fragmentShaderProgram] = createdFragmentShaderLibrary->CreateShaderProgram(
            ShaderStage::Fragment,
            "fp_main");

        if(fragmentShaderProgramCreateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->fragmentShader = std::move(fragmentShaderProgram);

        lightingResources->lightsUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&lightingResources->lightsUniformBufferBinding);

        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);

        lightingResources->baseColorTextureBinding.OfType(ResourceType::Texture);
        lightingResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);
        lightingResources->normalMapTextureBinding.OfType(ResourceType::Texture);
        lightingResources->normalMapSamplerBinding.OfType(ResourceType::Sampler);
        lightingResources->depthStencilTextureBinding.OfType(ResourceType::Texture);
        lightingResources->depthStencilSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
        gbufferTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(3)
            .AddBinding(&lightingResources->baseColorTextureBinding)
            .AddBinding(&lightingResources->normalMapTextureBinding)
            .AddBinding(&lightingResources->depthStencilTextureBinding);

        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo gbufferSamplerDescriptorSetLayoutCreateInfo;
        gbufferSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(3)
            .AddBinding(&lightingResources->baseColorSamplerBinding)
            .AddBinding(&lightingResources->normalMapSamplerBinding)
            .AddBinding(&lightingResources->depthStencilSamplerBinding);

        auto[createGBufferSamplerDescriptorSetLayoutResult, createdGBufferSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferSamplerDescriptorSetLayoutCreateInfo);
        if (createGBufferSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->gbufferSamplerDescriptorSetLayout = std::move(createdGBufferSamplerDescriptorSetLayout);

        SamplerCreateInfo samplerCreateInfo;

        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createBaseColorSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->baseColorSampler = std::move(createdBaseColorSampler);

        auto[createNormalMapSamplerResult, createdNormalMapSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createNormalMapSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->normalMapSampler = std::move(createdNormalMapSampler);

        auto[createDepthSamplerResult, createdDepthSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createDepthSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->depthBufferSampler = std::move(createdDepthSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, resources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, lightingResources->lightsBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, lightingResources->gbufferTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, lightingResources->gbufferSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->pipelineLayout = std::move(createdPipelineLayout);

        lightingResources->colorAttachmentTemplate.format = context->swapchain->GetInfo().format;
        lightingResources->colorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        lightingResources->colorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;

        // TODO result
        lightingResources->pipelineState = CreateLightingPipelineState(lightingResources.get());

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::VertexBuffer;

        auto[createQuadBufferResult, createdQuadBuffer] = context->device->CreateBuffer(
            bufferCreateInfo,
            fullscreenQuadVertices.data());

        if (createQuadBufferResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->fullscreenQuadBuffer = std::move(createdQuadBuffer);

        auto [allocateTextureSetResult, allocatedTextureSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
                lightingResources->gbufferTextureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        lightingResources->gbufferTextureDescriptorSet = std::move(allocatedTextureSet);

        auto [allocateSamplerSetResult, allocatedSamplerSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
            lightingResources->gbufferSamplerDescriptorSetLayout);

        if (allocateSamplerSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        lightingResources->gbufferSamplerDescriptorSet = std::move(allocatedSamplerSet);

        lightingResources->gbufferTextureDescriptorSet->WriteTexture(
            lightingResources->baseColorTextureBinding,
            gbufferPassResources->offscreen.baseColorTexture);

        lightingResources->gbufferTextureDescriptorSet->WriteTexture(
            lightingResources->normalMapTextureBinding,
            gbufferPassResources->offscreen.normalMapTexture);

        lightingResources->gbufferTextureDescriptorSet->WriteTexture(
            lightingResources->depthStencilTextureBinding,
            gbufferPassResources->offscreen.depthStencilBuffer);

        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
            lightingResources->baseColorSamplerBinding,
            gbufferPassResources->offscreen.baseColorSampler);

        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
            lightingResources->normalMapSamplerBinding,
            gbufferPassResources->offscreen.normalMapSampler);

        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
            lightingResources->depthStencilSamplerBinding,
            gbufferPassResources->offscreen.depthStencilSampler);

        return { true, std::move(lightingResources) };
    }

    ResultValue<bool, OffscreenTextures> DeferredRenderer::CreateOffscreenTextures()
    {
        OffscreenTextures textures;

        const auto& swapchainCreateInfo = context->swapchain->GetInfo();
        int32 textureWidth = swapchainCreateInfo.width;
        int32 textureHeight = swapchainCreateInfo.height;

        TextureCreateInfo baseColorTextureCreateInfo;
        baseColorTextureCreateInfo.format = baseColorFormat;
        baseColorTextureCreateInfo.width = textureWidth;
        baseColorTextureCreateInfo.height = textureHeight;
        baseColorTextureCreateInfo.usage =
              TextureUsageFlags::ColorAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        SamplerCreateInfo baseColorSamplerCreateInfo;

        auto[createBaseColorTextureResult, createdBaseColorTexture] = context->device->CreateTexture(baseColorTextureCreateInfo);
        if (createBaseColorTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(baseColorSamplerCreateInfo);
        if (createBaseColorSamplerResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures.baseColorTexture = std::move(createdBaseColorTexture);
        textures.baseColorSampler = std::move(createdBaseColorSampler);

        // Normal map

        TextureCreateInfo normalMapCreateInfo;
        normalMapCreateInfo.format = normalMapFormat;
        normalMapCreateInfo.width = textureWidth;
        normalMapCreateInfo.height = textureHeight;
        normalMapCreateInfo.usage =
              TextureUsageFlags::ColorAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        SamplerCreateInfo normalMapSamplerCreateInfo;

        auto[createNormapMapTextureResult, createdNormapMapTexture] = context->device->CreateTexture(normalMapCreateInfo);
        if (createNormapMapTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        auto[createNormapMapSamplerResult, createdNormapMapSampler] = context->device->CreateSampler(normalMapSamplerCreateInfo);
        if (createNormapMapTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures.normalMapTexture = std::move(createdNormapMapTexture);
        textures.normalMapSampler = std::move(createdNormapMapSampler);

        TextureCreateInfo depthBufferCreateInfo;
        depthBufferCreateInfo.format = depthStencilFormat;
        depthBufferCreateInfo.width = textureWidth;
        depthBufferCreateInfo.height = textureHeight;
        depthBufferCreateInfo.usage =
              TextureUsageFlags::DepthStencilAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        SamplerCreateInfo depthBufferSamplerCreateInfo;

        auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = context->device->CreateTexture(depthBufferCreateInfo);
        if (createDepthStencilBufferResult != GraphicsResult::Success)
        {
            return { false };
        }

        auto[createDepthStencilSamplerResult, createdDepthStencilSampler] = context->device->CreateSampler(depthBufferSamplerCreateInfo);
        if (createDepthStencilSamplerResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures.depthStencilBuffer = std::move(createdDepthStencilBuffer);
        textures.depthStencilSampler = std::move(createdDepthStencilSampler);

        return { true, textures };
    }
}
