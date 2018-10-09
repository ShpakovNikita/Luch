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

namespace Husky::Render::Deferred
{
    using namespace Graphics;

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
    static const Vector<QuadVertex> fullscreenQuadVertices =
    {
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        { {+3.0f, -1.0f, 0.0f}, {2.0f, 1.0f}},
        { {-1.0f, +3.0f, 0.0f}, {0.0f, -1.0f}},
    };

    // Think about passing these numbers through shader defines
    static const Map<SceneV1::AttributeSemantic, int32> SemanticToLocation =
    {
        { SceneV1::AttributeSemantic::Position, 0 },
        { SceneV1::AttributeSemantic::Normal, 1 },
        { SceneV1::AttributeSemantic::Tangent, 2 },
        { SceneV1::AttributeSemantic::Texcoord_0, 3 },
        { SceneV1::AttributeSemantic::Texcoord_1, 4 },
        { SceneV1::AttributeSemantic::Color_0, 5 },
    };

    static const Map<SceneV1::AttributeSemantic, DeferredShaderDefines> SemanticToFlag =
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

    DeferredRenderer::DeferredRenderer(
        const RefPtr<PhysicalDevice>& physicalDevice,
        const RefPtr<Surface>& surface,
        int32 aWidth,
        int32 aHeight)
        : width(aWidth)
        , height(aHeight)
    {
        context = MakeUnique<DeferredRendererContext>();
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

        auto[createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if (createCommandPoolResult != GraphicsResult::Success)
        {
            // TODO
            return false;
        }

        context->commandPool = std::move(createdCommandPool);

//        auto[swapchainChooseCreateInfoResult, swapchainCreateInfo] = Swapchain::ChooseSwapchainCreateInfo(width, height, context->physicalDevice, context->surface);
//        if (swapchainChooseCreateInfoResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return false;
//        }

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

        return true;
    }

    bool DeferredRenderer::Deinitialize()
    {
        context.release();
        return true;
    }

    ResultValue<bool, DeferredPreparedScene> DeferredRenderer::PrepareScene(const RefPtr<SceneV1::Scene>& scene)
    {
        DeferredPreparedScene preparedScene;

        preparedScene.scene = scene;

        const auto& sceneProperties = scene->GetSceneProperties();

        int32 perCameraUBOCount = 1;

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, perCameraUBOCount + 1 },
        };
        descriptorPoolCreateInfo.maxDescriptorSets = 1;

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        preparedScene.descriptorPool = std::move(createdDescriptorPool);

        auto [gBufferResourcesPrepared, preparedGBufferResources] = PrepareGBufferPassResources(preparedScene);
        if (gBufferResourcesPrepared)
        {
            preparedScene.gbuffer = std::move(preparedGBufferResources);
        }
        else
        {
            return { false };
        }

        auto[lightingResourcesPrepared, preparedLightingResources] = PrepareLightingPassResources(preparedScene);
        if (lightingResourcesPrepared)
        {
            preparedScene.lighting = std::move(preparedLightingResources);
        }
        else
        {
            return { false };
        }

        preparedScene.options.SetShadowMappingEnabled(true);

        auto[shadowMappingResourcesPrepared, shadowMappingResources] = PrepareShadowMappingPassResources(preparedScene);
        if (shadowMappingResourcesPrepared)
        {
            preparedScene.shadowMapping = std::move(shadowMappingResources);
        }
        else
        {
            return { false };
        }

        const auto& swapchainInfo = context->swapchain->GetInfo();
        preparedScene.frameResources.reserve(swapchainInfo.imageCount);

        auto[createdCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if (createdCommandPoolResult != GraphicsResult::Success)
        {
            // TODO
            return { false };
        }

        preparedScene.commandPool = std::move(createdCommandPool);

        for (int32 i = 0; i < swapchainInfo.imageCount; i++)
        {
            auto& frameResources = preparedScene.frameResources.emplace_back();

            auto [allocateLightingTextureDescriptorSetResult, allocatedLightingTextureDescriptorSet] =
                preparedScene.lighting.descriptorPool->AllocateDescriptorSet(preparedScene.lighting.gbufferTextureDescriptorSetLayout);
            if (allocateLightingTextureDescriptorSetResult != GraphicsResult::Success)
            {
                return { false };
            }

            frameResources.gbufferTextureDescriptorSet = std::move(allocatedLightingTextureDescriptorSet);

            auto [allocateLightingSamplerDescriptorSetResult, allocatedLightingSamplerDescriptorSet] =
                preparedScene.lighting.descriptorPool->AllocateDescriptorSet(preparedScene.lighting.gbufferSamplerDescriptorSetLayout);
            if (allocateLightingSamplerDescriptorSetResult != GraphicsResult::Success)
            {
                return { false };
            }

            frameResources.gbufferSamplerDescriptorSet = std::move(allocatedLightingSamplerDescriptorSet);

            auto[createFrameCommandPoolResult, createdFrameCommandPool] = context->commandQueue->CreateCommandPool();
            if (createFrameCommandPoolResult != GraphicsResult::Success)
            {
                return { false };
            }

            frameResources.commandPool = std::move(createdFrameCommandPool);

            auto [offscreenTexturesCreated, createdOffscreenTextures] = CreateOffscreenTextures();
            if (!offscreenTexturesCreated)
            {
                return { false };
            }

            frameResources.offscreen = std::move(createdOffscreenTextures);

            frameResources.gbufferTextureDescriptorSet->WriteTexture(
                &preparedScene.lighting.baseColorTextureBinding,
                frameResources.offscreen.baseColorTexture);

            frameResources.gbufferTextureDescriptorSet->WriteTexture(
                &preparedScene.lighting.normalMapTextureBinding,
                frameResources.offscreen.normalMapTexture);

            frameResources.gbufferTextureDescriptorSet->WriteTexture(
                &preparedScene.lighting.depthStencilTextureBinding,
                frameResources.offscreen.depthStencilBuffer);

            frameResources.gbufferSamplerDescriptorSet->WriteSampler(
                &preparedScene.lighting.baseColorSamplerBinding,
                frameResources.offscreen.baseColorSampler);

            frameResources.gbufferSamplerDescriptorSet->WriteSampler(
                &preparedScene.lighting.normalMapSamplerBinding,
                frameResources.offscreen.normalMapSampler);

            frameResources.gbufferSamplerDescriptorSet->WriteSampler(
                &preparedScene.lighting.depthStencilSamplerBinding,
                frameResources.offscreen.depthStencilSampler);
        }

        const auto& textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            texturesVector.push_back(texture);
        }

        TextureUploader textureUploader{ context->device, preparedScene.commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);

        if(!uploadTexturesSucceeded)
        {
            return { false };
        }

        for (const auto& commandList : uploadTexturesResult.commandLists)
        {
            context->commandQueue->Submit(commandList);
        }

        const auto& buffers = sceneProperties.buffers;
        for(const auto& buffer : buffers)
        {
            if(!buffer->UploadToDevice(context->device))
            {
                return { false };
            }
        }

        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            if (node->GetCamera() != nullptr)
            {
                HUSKY_ASSERT_MSG(preparedScene.cameraNode == nullptr, "Only one camera node is allowed");
                preparedScene.cameraNode = node;
                PrepareCameraNode(node, preparedScene);
            }
            else if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node, preparedScene);
            }
            else if (node->GetLight() != nullptr)
            {
                PrepareLightNode(node, preparedScene);
            }
        }

        for (const auto& material : sceneProperties.materials)
        {
            PrepareMaterial(material, preparedScene);
        }

        PrepareLights(preparedScene);

        return { true, preparedScene };
    }

    void DeferredRenderer::UpdateScene(DeferredPreparedScene& scene)
    {
        Mat4x4 identity = glm::mat4(1.0f);
        for (const auto& node : scene.scene->GetNodes())
        {
            UpdateNode(node, identity, scene);
        }
    }

    void DeferredRenderer::DrawScene(const DeferredPreparedScene& scene)
    {
        auto[acquireResult, acquiredTexture] = context->swapchain->GetNextAvailableTexture(nullptr);
        HUSKY_ASSERT(acquireResult == GraphicsResult::Success);

        auto& frameResource = scene.frameResources[acquiredTexture.index];

        auto [allocateGBufferCommandListResult, gBufferCmdList] = frameResource.commandPool->AllocateGraphicsCommandList();
        HUSKY_ASSERT(allocateGBufferCommandListResult == GraphicsResult::Success);

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport { 0, 0, (float32)framebufferWidth, (float32)framebufferHeight, 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        ColorAttachment baseColorAttachment = scene.gbuffer.baseColorAttachmentTemplate;
        baseColorAttachment.output.texture = frameResource.offscreen.baseColorTexture;

        ColorAttachment normalMapAttachment = scene.gbuffer.normalMapAttachmentTemplate;
        normalMapAttachment.output.texture = frameResource.offscreen.normalMapTexture;

        DepthStencilAttachment depthStencilAttachment = scene.gbuffer.depthStencilAttachmentTemplate;
        depthStencilAttachment.output.texture = frameResource.offscreen.depthStencilBuffer;

        RenderPassCreateInfo gBufferRenderPassCreateInfo;
        gBufferRenderPassCreateInfo
            .WithNColorAttachments(2)
            .AddColorAttachment(&baseColorAttachment)
            .AddColorAttachment(&normalMapAttachment)
            .WithDepthStencilAttachment(&depthStencilAttachment);

        auto[createGBufferRenderPassResult, createdGBufferRenderPass] = context->device->CreateRenderPass(gBufferRenderPassCreateInfo);
        HUSKY_ASSERT(createGBufferRenderPassResult == GraphicsResult::Success);

        gBufferCmdList->Begin(gBufferRenderPassCreateInfo);
        gBufferCmdList->SetViewports({viewport});
        gBufferCmdList->SetScissorRects({scissorRect});
        gBufferCmdList->BindBufferDescriptorSet(ShaderStage::Vertex, scene.gbuffer.pipelineLayout, scene.cameraNode->GetCamera()->GetVertexDescriptorSet());

        for (const auto& node : scene.scene->GetNodes())
        {
            DrawNode(node, scene, gBufferCmdList);
        }

        gBufferCmdList->End();

        context->commandQueue->Submit(gBufferCmdList);

        auto [allocateLightingCommandListResult, lightingCmdList] = frameResource.commandPool->AllocateGraphicsCommandList();
        HUSKY_ASSERT(allocateLightingCommandListResult == GraphicsResult::Success);

        ColorAttachment colorAttachment = scene.lighting.colorAttachmentTemplate;
        colorAttachment.output.texture = acquiredTexture.texture;

        RenderPassCreateInfo lightingRenderPassCreateInfo;
        lightingRenderPassCreateInfo
            .WithNColorAttachments(1)
            .AddColorAttachment(&colorAttachment);

        lightingCmdList->Begin(lightingRenderPassCreateInfo);
        lightingCmdList->BindPipelineState(scene.lighting.pipelineState);
        lightingCmdList->SetViewports({ viewport });
        lightingCmdList->SetScissorRects({ scissorRect });
        lightingCmdList->BindTextureDescriptorSet(ShaderStage::Fragment, scene.lighting.pipelineLayout, frameResource.gbufferTextureDescriptorSet);
        lightingCmdList->BindSamplerDescriptorSet(ShaderStage::Fragment, scene.lighting.pipelineLayout, frameResource.gbufferSamplerDescriptorSet);
        lightingCmdList->BindBufferDescriptorSet(ShaderStage::Fragment, scene.lighting.pipelineLayout, scene.cameraNode->GetCamera()->GetFragmentDescriptorSet());
        lightingCmdList->BindBufferDescriptorSet(ShaderStage::Fragment, scene.lighting.pipelineLayout, scene.lighting.lightsBufferDescriptorSet);

        lightingCmdList->BindVertexBuffers({ scene.lighting.fullscreenQuadBuffer }, {0}, 0);
        lightingCmdList->Draw(0, fullscreenQuadVertices.size());
        lightingCmdList->End();

        context->commandQueue->Submit(lightingCmdList);
        context->commandQueue->Present(acquiredTexture.index, context->swapchain);
    }

    void DeferredRenderer::PrepareCameraNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
        const auto& camera = node->GetCamera();

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = sizeof(CameraUniformBuffer);
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, buffer] = context->device->CreateBuffer(bufferCreateInfo);
        HUSKY_ASSERT(createBufferResult == GraphicsResult::Success);

        auto[mapMemoryResult, mappedMemory] = buffer->MapMemory(sizeof(CameraUniformBuffer), 0);
        HUSKY_ASSERT(mapMemoryResult == GraphicsResult::Success);

        auto[createVertexDescriptorSetResult, vertexDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.gbuffer.cameraBufferDescriptorSetLayout);
        HUSKY_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);

        auto[createFragmentDescriptorSetResult, fragmentDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.lighting.cameraBufferDescriptorSetLayout);
        HUSKY_ASSERT(createFragmentDescriptorSetResult == GraphicsResult::Success);

        camera->SetUniformBuffer(buffer);

        vertexDescriptorSet->WriteUniformBuffer(&scene.gbuffer.cameraUniformBufferBinding, buffer );
        fragmentDescriptorSet->WriteUniformBuffer(&scene.lighting.cameraUniformBufferBinding, buffer );

        vertexDescriptorSet->Update();
        fragmentDescriptorSet->Update();

        camera->SetVertexDescriptorSet(vertexDescriptorSet);
        camera->SetFragmentDescriptorSet(fragmentDescriptorSet);
    }

    void DeferredRenderer::PrepareMeshNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh, scene);
        }

        // TODO sort out node hierarchy
        for (const auto& child : node->GetChildren())
        {
            PrepareMeshNode(child, scene);
        }
    }

    void DeferredRenderer::PrepareLightNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
        const auto& light = node->GetLight();

        if (light!= nullptr)
        {
            PrepareLight(light, scene);
        }

        HUSKY_ASSERT_MSG(node->GetChildren().empty(), "Don't add children to light nodes");
    }

    void DeferredRenderer::PrepareNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
    }

    void DeferredRenderer::PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, DeferredPreparedScene& scene)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            RefPtr<PipelineState> pipelineState = primitive->GetPipelineState();
            if (pipelineState == nullptr)
            {
                pipelineState = CreateGBufferPipelineState(primitive, scene);
                primitive->SetPipelineState(pipelineState);
            }
        }

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = sizeof(MeshUniformBuffer);
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto[createUniformBufferResult, createdUniformBuffer] = context->device->CreateBuffer(bufferCreateInfo);

        HUSKY_ASSERT(createUniformBufferResult == GraphicsResult::Success);

        auto[mapMemoryResult, mappedMemory] = createdUniformBuffer->MapMemory(sizeof(MeshUniformBuffer), 0);
        HUSKY_ASSERT(mapMemoryResult == GraphicsResult::Success);

        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = scene.gbuffer.descriptorPool->AllocateDescriptorSet(scene.gbuffer.meshBufferDescriptorSetLayout);
        HUSKY_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);

        mesh->SetUniformBuffer(createdUniformBuffer);
        mesh->SetBufferDescriptorSet(allocatedDescriptorSet);

        allocatedDescriptorSet->WriteUniformBuffer(&scene.gbuffer.meshUniformBufferBinding, createdUniformBuffer );
        allocatedDescriptorSet->Update();
    }

    void DeferredRenderer::PrepareLight(const RefPtr<SceneV1::Light>& light, DeferredPreparedScene& scene)
    {
        scene.lights.push_back(light);
    }

    void DeferredRenderer::PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& material, DeferredPreparedScene& scene)
    {
        auto[allocateTextureDescriptorSetResult, textureDescriptorSet] = scene.gbuffer.descriptorPool->AllocateDescriptorSet(scene.gbuffer.materialTextureDescriptorSetLayout);
        HUSKY_ASSERT(allocateTextureDescriptorSetResult == GraphicsResult::Success);

        auto[allocateBufferDescriptorSetResult, bufferDescriptorSet] = scene.gbuffer.descriptorPool->AllocateDescriptorSet(scene.gbuffer.materialBufferDescriptorSetLayout);
        HUSKY_ASSERT(allocateBufferDescriptorSetResult == GraphicsResult::Success);

        auto[allocateSamplerDescriptorSetResult, samplerDescriptorSet] = scene.gbuffer.descriptorPool->AllocateDescriptorSet(scene.gbuffer.materialSamplerDescriptorSetLayout);
        HUSKY_ASSERT(allocateSamplerDescriptorSetResult == GraphicsResult::Success);

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = sizeof(MaterialUniform);
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        MaterialUniform materialUniform = GetMaterialUniform(material);

        auto[createBufferResult, createdBuffer] = context->device->CreateBuffer(bufferCreateInfo, &materialUniform);
        HUSKY_ASSERT(createBufferResult == GraphicsResult::Success);

        material->SetDeviceBuffer(createdBuffer);

        bufferDescriptorSet->WriteUniformBuffer(&scene.gbuffer.materialUniformBufferBinding, createdBuffer);

        if (material->HasBaseColorTexture())
        {
            textureDescriptorSet->WriteTexture(
                &scene.gbuffer.baseColorTextureBinding,
                material->metallicRoughness.baseColorTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                &scene.gbuffer.baseColorSamplerBinding,
                material->metallicRoughness.baseColorTexture.texture->GetDeviceSampler());
        }

        if (material->HasMetallicRoughnessTexture())
        {
            textureDescriptorSet->WriteTexture(
                &scene.gbuffer.metallicRoughnessTextureBinding,
                material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                &scene.gbuffer.metallicRoughnessSamplerBinding,
                material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceSampler());
        }

        if (material->HasNormalTexture())
        {
            textureDescriptorSet->WriteTexture(
                &scene.gbuffer.normalTextureBinding,
                material->normalTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                &scene.gbuffer.normalSamplerBinding,
                material->normalTexture.texture->GetDeviceSampler());
        }

        if (material->HasOcclusionTexture())
        {
            textureDescriptorSet->WriteTexture(
                &scene.gbuffer.occlusionTextureBinding,
                material->occlusionTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                &scene.gbuffer.occlusionSamplerBinding,
                material->occlusionTexture.texture->GetDeviceSampler());
        }

        if (material->HasEmissiveTexture())
        {
            textureDescriptorSet->WriteTexture(
                &scene.gbuffer.emissiveTextureBinding,
                material->emissiveTexture.texture->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                &scene.gbuffer.emissiveSamplerBinding,
                material->emissiveTexture.texture->GetDeviceSampler());
        }

        textureDescriptorSet->Update();
        samplerDescriptorSet->Update();
        bufferDescriptorSet->Update();

        material->SetTextureDescriptorSet(textureDescriptorSet);
        material->SetBufferDescriptorSet(bufferDescriptorSet);
        material->SetSamplerDescriptorSet(samplerDescriptorSet);
    }

    void DeferredRenderer::PrepareLights(DeferredPreparedScene& scene)
    {
        constexpr int MAX_LIGHTS_COUNT = 8;
        int32 lightsBufferSize = sizeof(LightUniform) * MAX_LIGHTS_COUNT;

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = lightsBufferSize;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto[createLightsBufferResult, createdLightsBuffer] = context->device->CreateBuffer(bufferCreateInfo);

        auto[mapMemoryResult, mappedMemory] = createdLightsBuffer->MapMemory(lightsBufferSize, 0);
        HUSKY_ASSERT(mapMemoryResult == GraphicsResult::Success);

        memset(mappedMemory, 0, lightsBufferSize);

        HUSKY_ASSERT(createLightsBufferResult == GraphicsResult::Success);
        scene.lighting.lightsBuffer = createdLightsBuffer;

        auto[createDescriptorSetResult, createdDescriptorSet] = scene.lighting.descriptorPool->AllocateDescriptorSet(scene.lighting.lightsBufferDescriptorSetLayout);
        HUSKY_ASSERT(createDescriptorSetResult == GraphicsResult::Success);
        scene.lighting.lightsBufferDescriptorSet = createdDescriptorSet;

        createdDescriptorSet->WriteUniformBuffer(&scene.lighting.lightsUniformBufferBinding, createdLightsBuffer);
        createdDescriptorSet->Update();
    }

    void DeferredRenderer::UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform, DeferredPreparedScene& scene)
    {
        const auto& mesh = node->GetMesh();

        Mat4x4 localTransformMatrix;
        const auto& localTransform = node->GetTransform();
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

        Mat4x4 transform = parentTransform * localTransformMatrix;

        if (mesh != nullptr)
        {
            UpdateMesh(mesh, transform, scene);
        }

        const auto& camera = node->GetCamera();

        if (camera != nullptr)
        {
            UpdateCamera(camera, transform, scene);
        }

        const auto& light = node->GetLight();
        if (light != nullptr)
        {
            UpdateLight(light, transform, scene);
        }

        for (const auto& child : node->GetChildren())
        {
            UpdateNode(child, transform, scene);
        }
    }

    void DeferredRenderer::UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform, DeferredPreparedScene& scene)
    {
        const auto& buffer = mesh->GetUniformBuffer();

        MeshUniformBuffer meshUniformBuffer;
        meshUniformBuffer.transform = transform;
        meshUniformBuffer.inverseTransform = glm::inverse(transform);

        // TODO template function member in buffer to write updates
        memcpy(buffer->GetMappedMemory(), &meshUniformBuffer, sizeof(MeshUniformBuffer));
    }

    void DeferredRenderer::UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform, DeferredPreparedScene& scene)
    {
        const auto& buffer = camera->GetUniformBuffer();

        camera->SetCameraViewMatrix(glm::inverse(transform));

        CameraUniformBuffer cameraUniformBuffer;
        cameraUniformBuffer.view = camera->GetCameraViewMatrix();
        cameraUniformBuffer.inverseView = glm::inverse(cameraUniformBuffer.view);
        cameraUniformBuffer.projection = camera->GetCameraProjectionMatrix();
        cameraUniformBuffer.inverseProjection = glm::inverse(cameraUniformBuffer.projection);
        cameraUniformBuffer.inverseProjection = glm::inverse(cameraUniformBuffer.projection);
        cameraUniformBuffer.viewProjection = cameraUniformBuffer.projection * cameraUniformBuffer.view;
        cameraUniformBuffer.inverseViewProjection = glm::inverse(cameraUniformBuffer.viewProjection);
        cameraUniformBuffer.position = Vec4{ camera->GetCameraPosition(), 1.0 };

        Vec2 zMinMax;
        auto cameraType = camera->GetCameraType();
        if (cameraType == SceneV1::CameraType::Orthographic)
        {
            auto orthographicCamera = (SceneV1::OrthographicCamera*)camera.Get();
            zMinMax.x = orthographicCamera->GetZNear();
            zMinMax.y = orthographicCamera->GetZFar();
        }
        else if(cameraType == SceneV1::CameraType::Perspective)
        {
            auto perspectiveCamera = (SceneV1::PerspectiveCamera*)camera.Get();
            zMinMax.x = perspectiveCamera->GetZNear();
            zMinMax.y = perspectiveCamera->GetZFar().value_or(Limits<float>::infinity());
        }

        cameraUniformBuffer.zMinMax = zMinMax;

        // TODO
        memcpy(buffer->GetMappedMemory(), &cameraUniformBuffer, sizeof(CameraUniformBuffer));
    }

    void DeferredRenderer::UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform, DeferredPreparedScene& scene)
    {
        auto viewTransform = scene.cameraNode->GetCamera()->GetCameraViewMatrix();

        LightUniform lightUniform;

        lightUniform.positionWS = transform * Vec4{ 0.0, 0.0, 0.0, 1.0 };
        lightUniform.directionWS = transform * Vec4{ light->GetDirection().value_or(Vec3{0, 0, 1}), 0.0 };
        lightUniform.positionVS = viewTransform * lightUniform.positionWS;
        lightUniform.directionVS = viewTransform * lightUniform.directionWS;
        lightUniform.color = Vec4{ light->GetColor().value_or(Vec3{1.0, 1.0, 1.0}), 1.0 };
        lightUniform.enabled = light->IsEnabled() ? 1 : 0;
        lightUniform.type = static_cast<int32>(light->GetType());
        lightUniform.spotlightAngle = light->GetSpotlightAngle().value_or(0.0);
        lightUniform.range = light->GetRange().value_or(0.0);
        lightUniform.intensity = light->GetIntensity();
        lightUniform.enabled = true;

        int32 offset = sizeof(LightUniform)*light->GetIndex();
        memcpy((Byte*)scene.lighting.lightsBuffer->GetMappedMemory() + offset, &lightUniform, sizeof(LightUniform));
    }

    void DeferredRenderer::DrawNode(const RefPtr<SceneV1::Node>& node, const DeferredPreparedScene& scene, GraphicsCommandList* commandList)
    {
        const auto& mesh = node->GetMesh();
        if (mesh != nullptr)
        {
            DrawMesh(mesh, scene, commandList);
        }

        for (const auto& child : node->GetChildren())
        {
            DrawNode(child, scene, commandList);
        }
    }

    void DeferredRenderer::DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, const DeferredPreparedScene& scene, GraphicsCommandList* commandList)
    {
        // TODO descriptor set indices
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            scene.gbuffer.pipelineLayout,
            mesh->GetBufferDescriptorSet());

        for (const auto& primitive : mesh->GetPrimitives())
        {
            if (primitive->GetMaterial()->alphaMode != SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, scene, commandList);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            if (primitive->GetMaterial()->alphaMode == SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, scene, commandList);
            }
        }
    }

    void DeferredRenderer::DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, const DeferredPreparedScene& scene, GraphicsCommandList* commandList)
    {
        auto& pipelineState = primitive->GetPipelineState();

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

        const auto& material = primitive->GetMaterial();

        commandList->BindPipelineState(pipelineState);
        commandList->BindVertexBuffers(graphicsVertexBuffers, offsets, 0);
        commandList->BindTextureDescriptorSet(ShaderStage::Fragment, scene.gbuffer.pipelineLayout, material->GetTextureDescriptorSet());
        commandList->BindBufferDescriptorSet(ShaderStage::Fragment, scene.gbuffer.pipelineLayout, material->GetBufferDescriptorSet());
        commandList->BindSamplerDescriptorSet(ShaderStage::Fragment, scene.gbuffer.pipelineLayout, material->GetSamplerDescriptorSet());
        commandList->BindIndexBuffer(
                indexBuffer.backingBuffer->GetDeviceBuffer(),
                indexBuffer.indexType,
                indexBuffer.byteOffset);
        commandList->DrawIndexedInstanced(indexBuffer.count, 0, 1, 0);
    }

    MaterialUniform DeferredRenderer::GetMaterialUniform(SceneV1::PbrMaterial* material)
    {
        MaterialUniform materialUniform;
        materialUniform.baseColorFactor = material->metallicRoughness.baseColorFactor;
        materialUniform.metallicFactor = material->metallicRoughness.metallicFactor;
        materialUniform.roughnessFactor = material->metallicRoughness.roughnessFactor;
        materialUniform.normalScale = material->normalTexture.scale;
        materialUniform.occlusionStrength = material->occlusionTexture.strength;
        materialUniform.emissiveFactor = material->emissiveFactor;
        materialUniform.alphaCutoff = material->alphaCutoff;
        return materialUniform;
    }

    Vector<Byte> DeferredRenderer::LoadShaderSource(const FilePath& path)
    {
        FileStream fileStream{ path, FileOpenModes::Read };
        auto fileSize = fileStream.GetSize();
        Vector<Byte> result;
        result.resize(fileSize + 1); // +1 for null termination
        auto bytesRead = fileStream.Read(result.data(), fileSize, sizeof(Byte));
        HUSKY_ASSERT(bytesRead == fileSize);
        return result;
    }

    RefPtr<PipelineState> DeferredRenderer::CreateGBufferPipelineState(const RefPtr<SceneV1::Primitive>& primitive, DeferredPreparedScene& scene)
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
        ci.pipelineLayout = scene.gbuffer.pipelineLayout;

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

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = CreateShaderLibrary(
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/gbuffer_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/gbuffer.vert",
    #endif
#endif
            shaderDefines);

        if (!vertexShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
        }

        auto[vertexShaderCreateResult, createdVertexShader] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "vp_main");
        HUSKY_ASSERT(vertexShaderCreateResult == GraphicsResult::Success);

        auto vertexShader = std::move(createdVertexShader);

        auto[fragmentShaderLibraryCreated, fragmentShaderLibrary] = CreateShaderLibrary(
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/gbuffer_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/gbuffer.frag",
    #endif
#endif
            shaderDefines);

        if (!fragmentShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
        }

        auto[fragmentShaderCreateResult, createdFragmentShader] = fragmentShaderLibrary->CreateShaderProgram(ShaderStage::Fragment, "fp_main");
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

    RefPtr<PipelineState> DeferredRenderer::CreateLightingPipelineState(const LightingPassResources& lighting)
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
        ci.pipelineLayout = lighting.pipelineLayout;
        ci.vertexProgram = lighting.vertexShader;
        ci.fragmentProgram = lighting.fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, GBufferPassResources> DeferredRenderer::PrepareGBufferPassResources(DeferredPreparedScene& scene)
    {
        GBufferPassResources resources;

        const auto& sceneProperties = scene.scene->GetSceneProperties();

        const int32 texturesPerMaterial = 5;

        int32 textureDescriptorCount = sceneProperties.materials.size() * texturesPerMaterial;
        int32 perMeshUBOCount = sceneProperties.meshes.size();

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        // two sets per material, one set per mesh
        descriptorPoolCreateInfo.maxDescriptorSets = sceneProperties.materials.size() + sceneProperties.meshes.size();

        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::Texture, textureDescriptorCount + 1 },
            { ResourceType::UniformBuffer, perMeshUBOCount + 1 },
            { ResourceType::Sampler, textureDescriptorCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.descriptorPool = std::move(createdDescriptorPool);

        resources.materialUniformBufferBinding.OfType(ResourceType::UniformBuffer).AtStage(ShaderStage::Fragment);

        resources.meshUniformBufferBinding.OfType(ResourceType::UniformBuffer).AtStage(ShaderStage::Vertex);

        resources.baseColorTextureBinding.OfType(ResourceType::Texture).AtStage(ShaderStage::Fragment);
        resources.baseColorSamplerBinding.OfType(ResourceType::Sampler).AtStage(ShaderStage::Fragment);

        resources.metallicRoughnessTextureBinding.OfType(ResourceType::Texture).AtStage(ShaderStage::Fragment);
        resources.metallicRoughnessSamplerBinding.OfType(ResourceType::Sampler).AtStage(ShaderStage::Fragment);

        resources.normalTextureBinding.OfType(ResourceType::Texture).AtStage(ShaderStage::Fragment);
        resources.normalSamplerBinding.OfType(ResourceType::Sampler).AtStage(ShaderStage::Fragment);

        resources.occlusionTextureBinding.OfType(ResourceType::Texture).AtStage(ShaderStage::Fragment);
        resources.occlusionSamplerBinding.OfType(ResourceType::Sampler).AtStage(ShaderStage::Fragment);

        resources.emissiveTextureBinding.OfType(ResourceType::Texture).AtStage(ShaderStage::Fragment);
        resources.emissiveSamplerBinding.OfType(ResourceType::Sampler).AtStage(ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
        meshDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .AddBinding(&resources.meshUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialBufferDescriptorSetLayoutCreateInfo;
        materialBufferDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .AddBinding(&resources.materialUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialTextureDescriptorSetLayoutCreateInfo;
        materialTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(5)
            .AddBinding(&resources.baseColorTextureBinding)
            .AddBinding(&resources.metallicRoughnessTextureBinding)
            .AddBinding(&resources.normalTextureBinding)
            .AddBinding(&resources.occlusionTextureBinding)
            .AddBinding(&resources.emissiveTextureBinding);

        DescriptorSetLayoutCreateInfo materialSamplerDescriptorSetLayoutCreateInfo;
        materialSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(5)
            .AddBinding(&resources.baseColorSamplerBinding)
            .AddBinding(&resources.metallicRoughnessSamplerBinding)
            .AddBinding(&resources.normalSamplerBinding)
            .AddBinding(&resources.occlusionSamplerBinding)
            .AddBinding(&resources.emissiveSamplerBinding);

        auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(meshDescriptorSetLayoutCreateInfo);
        if (createMeshDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.meshBufferDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);

        auto[createMaterialTextureDescriptorSetLayoutResult, createdMaterialTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(materialTextureDescriptorSetLayoutCreateInfo);
        if (createMaterialTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.materialTextureDescriptorSetLayout = std::move(createdMaterialTextureDescriptorSetLayout);

        auto[createMaterialBufferDescriptorSetLayoutResult, createdMaterialBufferDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(materialBufferDescriptorSetLayoutCreateInfo);
        if (createMaterialBufferDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.materialBufferDescriptorSetLayout = std::move(createdMaterialBufferDescriptorSetLayout);

        auto[createMaterialSamplerDescriptorSetLayoutResult, createdMaterialSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(materialSamplerDescriptorSetLayoutCreateInfo);
        if (createMaterialSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.materialSamplerDescriptorSetLayout = std::move(createdMaterialSamplerDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources.cameraUniformBufferBinding);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cameraDescriptorSetLayoutCreateInfo);
        if (createCameraDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.cameraBufferDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Vertex, resources.cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Vertex, resources.meshBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resources.materialTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resources.materialBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resources.materialSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.pipelineLayout = std::move(createdPipelineLayout);

        resources.baseColorAttachmentTemplate.format = baseColorFormat;
        resources.baseColorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        resources.baseColorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
        resources.baseColorAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

        resources.normalMapAttachmentTemplate.format = normalMapFormat;
        resources.normalMapAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        resources.normalMapAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
        resources.normalMapAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };

        resources.depthStencilAttachmentTemplate.format = depthStencilFormat;
        resources.depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
        resources.depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::Store;
        resources.depthStencilAttachmentTemplate.stencilLoadOperation = AttachmentLoadOperation::Clear;
        resources.depthStencilAttachmentTemplate.stencilStoreOperation = AttachmentStoreOperation::Store;
        resources.depthStencilAttachmentTemplate.depthClearValue = maxDepth;
        resources.depthStencilAttachmentTemplate.stencilClearValue = 0xffffffff;

        return { true, resources };
    }

    ResultValue<bool, LightingPassResources> DeferredRenderer::PrepareLightingPassResources(DeferredPreparedScene& scene)
    {
        LightingPassResources resources;
        int32 swapchainLength = context->swapchain->GetInfo().imageCount;

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 1 + swapchainLength + 1;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, 1 },
            { ResourceType::Texture, swapchainLength * OffscreenImagesCount + 1 },
            { ResourceType::Sampler, swapchainLength * OffscreenImagesCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(descriptorPoolCreateInfo);
        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.descriptorPool = std::move(createdDescriptorPool);

        ShaderDefines<DeferredShaderDefines> shaderDefines;
        shaderDefines.mapping = &FlagToString;

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = CreateShaderLibrary(
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.vert",
    #endif
#endif
            shaderDefines);

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

        resources.vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = CreateShaderLibrary(
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.frag",
    #endif
#endif
            shaderDefines);

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

        resources.fragmentShader = std::move(fragmentShaderProgram);

        resources.lightsUniformBufferBinding
            .OfType(ResourceType::UniformBuffer)
            .AtStage(ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources.cameraUniformBufferBinding);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cameraDescriptorSetLayoutCreateInfo);
        if (createCameraDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.cameraBufferDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources.lightsUniformBufferBinding);

        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);

        resources.baseColorTextureBinding
            .OfType(ResourceType::Texture)
            .AtStage(ShaderStage::Fragment);

        resources.baseColorSamplerBinding
            .OfType(ResourceType::Sampler)
            .AtStage(ShaderStage::Fragment);

        resources.normalMapTextureBinding
            .OfType(ResourceType::Texture)
            .AtStage(ShaderStage::Fragment);

        resources.normalMapSamplerBinding
            .OfType(ResourceType::Sampler)
            .AtStage(ShaderStage::Fragment);

        resources.depthStencilTextureBinding
            .OfType(ResourceType::Texture)
            .AtStage(ShaderStage::Fragment);

        resources.depthStencilSamplerBinding
            .OfType(ResourceType::Sampler)
            .AtStage(ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
        gbufferTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(3)
            .AddBinding(&resources.baseColorTextureBinding)
            .AddBinding(&resources.normalMapTextureBinding)
            .AddBinding(&resources.depthStencilTextureBinding);

        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo gbufferSamplerDescriptorSetLayoutCreateInfo;
        gbufferSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(3)
            .AddBinding(&resources.baseColorSamplerBinding)
            .AddBinding(&resources.normalMapSamplerBinding)
            .AddBinding(&resources.depthStencilSamplerBinding);

        auto[createGBufferSamplerDescriptorSetLayoutResult, createdGBufferSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferSamplerDescriptorSetLayoutCreateInfo);
        if (createGBufferSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.gbufferSamplerDescriptorSetLayout = std::move(createdGBufferSamplerDescriptorSetLayout);

        SamplerCreateInfo samplerCreateInfo;

        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createBaseColorSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.baseColorSampler = std::move(createdBaseColorSampler);

        auto[createNormalMapSamplerResult, createdNormalMapSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createNormalMapSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.normalMapSampler = std::move(createdNormalMapSampler);

        auto[createDepthSamplerResult, createdDepthSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createDepthSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.depthBufferSampler = std::move(createdDepthSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, resources.cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resources.lightsBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resources.gbufferTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resources.gbufferSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.pipelineLayout = std::move(createdPipelineLayout);

        resources.colorAttachmentTemplate.format = context->swapchain->GetInfo().format;
        resources.colorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        resources.colorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;

        // TODO result
        resources.pipelineState = CreateLightingPipelineState(resources);

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::VertexBuffer;

        auto[createQuadBufferResult, createdQuadBuffer] = context->device->CreateBuffer(bufferCreateInfo, fullscreenQuadVertices.data());

        if (createQuadBufferResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.fullscreenQuadBuffer = std::move(createdQuadBuffer);

        return { true, resources };
    }

    ResultValue<bool, ShadowMappingPassResources> DeferredRenderer::PrepareShadowMappingPassResources(
        DeferredPreparedScene& scene)
    {
        const auto& shadowMappingOptions = scene.options.GetShadowMappingOptions();
        ShadowMappingPassResources resources;

        resources.cameraUniformBufferBinding
            .OfType(ResourceType::UniformBuffer)
            .AtStage(ShaderStage::Vertex);

        resources.meshUniformBufferBinding
            .OfType(ResourceType::UniformBuffer)
            .AtStage(ShaderStage::Vertex);

        resources.depthStencilAttachmentTemplate.format = shadowMappingOptions.shadowMapFormat;
        resources.depthStencilAttachmentTemplate.depthClearValue = 1.0f;
        resources.depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
        resources.depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::Store;

        DescriptorSetLayoutCreateInfo cameraBufferSetLayoutCreateInfo;
        cameraBufferSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources.cameraUniformBufferBinding);

        auto [cameraBufferSetLayoutCreateResult, createdCameraBufferSetLayout] = context->device->CreateDescriptorSetLayout(
            cameraBufferSetLayoutCreateInfo);
        if(cameraBufferSetLayoutCreateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.cameraBufferSetLayout = std::move(createdCameraBufferSetLayout);

        DescriptorSetLayoutCreateInfo meshBufferSetLayoutCreateInfo;
        meshBufferSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources.meshUniformBufferBinding);

        auto [meshBufferSetLayoutCreateResult, createdMeshBufferSetLayout] = context->device->CreateDescriptorSetLayout(
            meshBufferSetLayoutCreateInfo);
        if(meshBufferSetLayoutCreateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.meshBufferSetLayout = std::move(createdMeshBufferSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Vertex, resources.cameraBufferSetLayout)
            .AddSetLayout(ShaderStage::Vertex, resources.meshBufferSetLayout);

        auto [createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if(createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.pipelineLayout = std::move(createdPipelineLayout);

        VertexInputAttributeDescription positionAttribute;
        positionAttribute.format = Format::R32G32B32A32Sfloat;

        VertexInputBindingDescription vertexBufferBinding;
        vertexBufferBinding.stride = sizeof(Vertex);

        ShaderDefines<DeferredShaderDefines> shaderDefines;

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = CreateShaderLibrary(
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\todo.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/shadowmap_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/todo.vert",
    #endif
#endif
            shaderDefines);

        if(!vertexShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        auto [createVertexProgramResult, createdVertexProgram] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "vp_main");

        if(createVertexProgramResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        PipelineStateCreateInfo pipelineStateCreateInfo;

        pipelineStateCreateInfo.inputAssembler.attributes.push_back(positionAttribute);
        pipelineStateCreateInfo.inputAssembler.bindings.push_back(vertexBufferBinding);
        // todo primitive topology pipelineStateCreateInfo.inputAssembler
        pipelineStateCreateInfo.vertexProgram = createdVertexProgram;
        pipelineStateCreateInfo.depthStencil.depthTestEnable = true;
        pipelineStateCreateInfo.depthStencil.depthWriteEnable = true;
        pipelineStateCreateInfo.depthStencil.depthCompareFunction = CompareFunction::Less;
        pipelineStateCreateInfo.depthStencil.depthStencilFormat = Format::D32Sfloat;
        pipelineStateCreateInfo.pipelineLayout = resources.pipelineLayout;

        auto [createPipelineStateResult, createdPipelineState] = context->device->CreatePipelineState(pipelineStateCreateInfo);
        if(createPipelineStateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resources.pipelineState = std::move(createdPipelineState);

        return { true, resources };
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
              TextureUsageFlags::DethpStencilAttachment
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

    ResultValue<bool, RefPtr<ShaderLibrary>> DeferredRenderer::CreateShaderLibrary(
        const String& path,
        const ShaderDefines<DeferredShaderDefines>& defines)
    {
        auto shaderSource = LoadShaderSource(path);

        auto [createLibraryResult, library] = context->device->CreateShaderLibraryFromSource(shaderSource, defines.defines);
        if(createLibraryResult != GraphicsResult::Success && createLibraryResult != GraphicsResult::CompilerWarning)
        {
            HUSKY_ASSERT(false);
            return { false };
        }
        else
        {
            return { true, library };
        }
    }
}
