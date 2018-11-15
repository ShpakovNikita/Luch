#include <Luch/Render/Deferred/GBufferRenderer.h>
#include <Luch/Render/TextureUploader.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/Deferred/DeferredShaderDefines.h>
#include <Luch/Render/Deferred/GBufferPassResources.h>
#include <Luch/Render/Deferred/DeferredResources.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/AttributeSemantic.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/PipelineState.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/Graphics/PipelineStateCreateInfo.h>

#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Deferred/GBufferPassResources.h>
#include <Luch/Render/Deferred/GBufferTextures.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

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

    const String GBufferRenderer::RendererName{"GBuffer"};

    GBufferRenderer::GBufferRenderer() = default;
    GBufferRenderer::~GBufferRenderer() = default;

    bool GBufferRenderer::Initialize()
    {
        LUCH_ASSERT(context != nullptr);
        LUCH_ASSERT(commonResources != nullptr);

        Vector<Format> depthFormats =
        {
            Format::D32SfloatS8Uint,
            Format::D24UnormS8Uint,
            Format::D16UnormS8Uint,
        };

        auto supportedDepthFormats = context->physicalDevice->GetSupportedDepthStencilFormats(depthFormats);
        LUCH_ASSERT_MSG(!depthFormats.empty(), "No supported depth formats");
        depthStencilFormat = depthFormats.front();

        auto [gBufferResourcesPrepared, preparedGBufferResources] = PrepareGBufferPassResources();
        if (!gBufferResourcesPrepared)
        {
            return  false;
        }

        resources = std::move(preparedGBufferResources);

        auto [offscreenTexturesCreated, createdGBufferTextures] = CreateGBufferTextures();
        if (!offscreenTexturesCreated)
        {
            return false;
        }

        gbuffer = std::move(createdGBufferTextures);

        return true;
    }

    bool GBufferRenderer::Deinitialize()
    {
        context.reset();
        resources.reset();
        gbuffer.reset();

        return true;
    }

    void GBufferRenderer::PrepareScene(SceneV1::Scene* scene)
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

        LUCH_ASSERT(uploadTexturesSucceeded);

        for (const auto& commandList : uploadTexturesResult.commandLists)
        {
            context->commandQueue->Submit(commandList);
        }

        const auto& buffers = sceneProperties.buffers;
        for(const auto& buffer : buffers)
        {
            [[maybe_unused]] bool uploadSucceeded = buffer->UploadToDevice(context->device);
            LUCH_ASSERT(uploadSucceeded);
        }

        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node);
            }
        }

        for (const auto& material : sceneProperties.materials)
        {
            PrepareMaterial(material);
        }
    }

    void GBufferRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        resources->sharedBuffer->Reset();

        for (const auto& node : scene->GetNodes())
        {
            UpdateNode(node);
        }
    }

    GBufferTextures* GBufferRenderer::DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera)
    {
        auto [allocateCommandListResult, cmdList] = resources->commandPool->AllocateGraphicsCommandList();
        LUCH_ASSERT(allocateCommandListResult == GraphicsResult::Success);

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport {
            0, 0, static_cast<float32>(framebufferWidth), static_cast<float32>(framebufferHeight), 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        ColorAttachment baseColorAttachment = resources->baseColorAttachmentTemplate;
        baseColorAttachment.output.texture = gbuffer->baseColorTexture;

        ColorAttachment normalMapAttachment = resources->normalMapAttachmentTemplate;
        normalMapAttachment.output.texture = gbuffer->normalMapTexture;

        DepthStencilAttachment depthStencilAttachment = resources->depthStencilAttachmentTemplate;
        depthStencilAttachment.output.texture = gbuffer->depthStencilBuffer;

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
            .WithName(RendererName)
            .WithNColorAttachments(2)
            .AddColorAttachment(&baseColorAttachment)
            .AddColorAttachment(&normalMapAttachment)
            .WithDepthStencilAttachment(&depthStencilAttachment);

        auto[createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
        LUCH_ASSERT(createRenderPassResult == GraphicsResult::Success);

        cmdList->Begin();
        cmdList->BeginRenderPass(renderPassCreateInfo);
        cmdList->SetViewports({ viewport });
        cmdList->SetScissorRects({ scissorRect });
        cmdList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            resources->pipelineLayout,
            camera->GetDescriptorSet("Deferred"));

        for (const auto& node : scene->GetNodes())
        {
            DrawNode(node, cmdList);
        }

        cmdList->EndRenderPass();
        cmdList->End();

        context->commandQueue->Submit(cmdList);

        return gbuffer.get();
    }

    void GBufferRenderer::PrepareMeshNode(SceneV1::Node* node)
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

    void GBufferRenderer::PrepareNode(SceneV1::Node* node)
    {
    }

    void GBufferRenderer::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        RefPtr<PipelineState> pipelineState = primitive->GetPipelineState(RendererName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreateGBufferPipelineState(primitive);
            primitive->SetPipelineState(RendererName, pipelineState);
        }
    }

    void GBufferRenderer::PrepareMesh(SceneV1::Mesh* mesh)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            PreparePrimitive(primitive);
        }

        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->meshBufferDescriptorSetLayout);

        LUCH_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);

        mesh->SetBufferDescriptorSet(RendererName, allocatedDescriptorSet);
    }

    void GBufferRenderer::PrepareMaterial(SceneV1::PbrMaterial* material)
    {
        auto[allocateTextureDescriptorSetResult, textureDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->materialTextureDescriptorSetLayout);
        LUCH_ASSERT(allocateTextureDescriptorSetResult == GraphicsResult::Success);

        auto[allocateBufferDescriptorSetResult, bufferDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->materialBufferDescriptorSetLayout);
        LUCH_ASSERT(allocateBufferDescriptorSetResult == GraphicsResult::Success);

        auto[allocateSamplerDescriptorSetResult, samplerDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->materialSamplerDescriptorSetLayout);
        LUCH_ASSERT(allocateSamplerDescriptorSetResult == GraphicsResult::Success);

        material->SetTextureDescriptorSet(RendererName, textureDescriptorSet);
        material->SetBufferDescriptorSet(RendererName, bufferDescriptorSet);
        material->SetSamplerDescriptorSet(RendererName, samplerDescriptorSet);

        UpdateMaterial(material);
    }

    void GBufferRenderer::UpdateNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            UpdateMesh(mesh, node->GetWorldTransform());
        }

        for (const auto& child : node->GetChildren())
        {
            UpdateNode(child);
        }
    }

    void GBufferRenderer::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
    {
        MeshUniform meshUniform;
        meshUniform.transform = transform;
        meshUniform.inverseTransform = glm::inverse(transform);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MeshUniform), 16);

        memcpy(suballocation.offsetMemory, &meshUniform, sizeof(MeshUniform));

        auto descriptorSet = mesh->GetBufferDescriptorSet(RendererName);

        descriptorSet->WriteUniformBuffer(
            resources->meshUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        descriptorSet->Update();
    }

    void GBufferRenderer::UpdateMaterial(SceneV1::PbrMaterial* material)
    {
        auto textureDescriptorSet = material->GetTextureDescriptorSet(RendererName);
        auto samplerDescriptorSet = material->GetSamplerDescriptorSet(RendererName);
        auto bufferDescriptorSet = material->GetBufferDescriptorSet(RendererName);

        if (material->HasBaseColorTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->baseColorTextureBinding,
                material->GetBaseColorTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->baseColorSamplerBinding,
                material->GetBaseColorTexture()->GetDeviceSampler());
        }

        if (material->HasMetallicRoughnessTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->metallicRoughnessTextureBinding,
                material->GetMetallicRoughnessTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->metallicRoughnessSamplerBinding,
                material->GetMetallicRoughnessTexture()->GetDeviceSampler());
        }

        if (material->HasNormalTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->normalTextureBinding,
                material->GetNormalTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->normalSamplerBinding,
                material->GetNormalTexture()->GetDeviceSampler());
        }

        if (material->HasOcclusionTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->occlusionTextureBinding,
                material->GetOcclusionTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->occlusionSamplerBinding,
                material->GetOcclusionTexture()->GetDeviceSampler());
        }

        if (material->HasEmissiveTexture())
        {
            textureDescriptorSet->WriteTexture(
                resources->emissiveTextureBinding,
                material->GetEmissiveTexture()->GetDeviceTexture());

            samplerDescriptorSet->WriteSampler(
                resources->emissiveSamplerBinding,
                material->GetEmissiveTexture()->GetDeviceSampler());
        }

        MaterialUniform materialUniform = RenderUtils::GetMaterialUniform(material);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MaterialUniform), 16);

        bufferDescriptorSet->WriteUniformBuffer(
            resources->materialUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        memcpy(suballocation.offsetMemory, &materialUniform, sizeof(MaterialUniform));

        textureDescriptorSet->Update();
        samplerDescriptorSet->Update();
        bufferDescriptorSet->Update();
    }

    void GBufferRenderer::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
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

    void GBufferRenderer::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            resources->pipelineLayout,
            mesh->GetBufferDescriptorSet(RendererName));

        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->GetProperties().alphaMode != SceneV1::AlphaMode::Blend)
            {
                BindMaterial(material, commandList);
                DrawPrimitive(primitive, commandList);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Blend)
            {
                BindMaterial(material, commandList);
                DrawPrimitive(primitive, commandList);
            }
        }
    }

    void GBufferRenderer::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
    {
        commandList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            material->GetTextureDescriptorSet(RendererName));

        commandList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            material->GetBufferDescriptorSet(RendererName));

        commandList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            material->GetSamplerDescriptorSet(RendererName));
    }

    void GBufferRenderer::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
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

        LUCH_ASSERT(primitive->GetIndexBuffer().has_value());
        const auto& indexBuffer = *primitive->GetIndexBuffer();

        commandList->BindPipelineState(pipelineState);
        commandList->BindVertexBuffers(graphicsVertexBuffers, offsets, 0);

        commandList->BindIndexBuffer(
                indexBuffer.backingBuffer->GetDeviceBuffer(),
                indexBuffer.indexType,
                indexBuffer.byteOffset);

        commandList->DrawIndexedInstanced(indexBuffer.count, 0, 1, 0);
    }

    RefPtr<PipelineState> GBufferRenderer::CreateGBufferPipelineState(SceneV1::Primitive* primitive)
    {
        PipelineStateCreateInfo ci;

        ShaderDefines<DeferredShaderDefines> shaderDefines;
        shaderDefines.mapping = &FlagToString;

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        LUCH_ASSERT(vertexBuffers.size() == 1);

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

        if (material->GetProperties().doubleSided)
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

        baseColorAttachment.blendEnable = material->GetProperties().alphaMode == SceneV1::AlphaMode::Blend;
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
        ci.pipelineLayout = resources->pipelineLayout;

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

        if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Mask)
        {
            shaderDefines.AddFlag(DeferredShaderDefines::AlphaMask);
        }

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\gbuffer.vert",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/gbuffer_vp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/gbuffer.vert",
    #else
            "",
    #endif
#endif
            shaderDefines.defines);

        if (!vertexShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
        }

        auto[vertexShaderCreateResult, createdVertexShader] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "vp_main");
        LUCH_ASSERT(vertexShaderCreateResult == GraphicsResult::Success);

        auto vertexShader = std::move(createdVertexShader);

        auto[fragmentShaderLibraryCreated, fragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\gbuffer.frag",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/gbuffer_fp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/gbuffer.frag",
    #else
            "",
    #endif
#endif
            shaderDefines.defines);

        if (!fragmentShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
        }

        auto[fragmentShaderCreateResult, createdFragmentShader] = fragmentShaderLibrary->CreateShaderProgram(
            ShaderStage::Fragment,
            "fp_main");
        LUCH_ASSERT(fragmentShaderCreateResult == GraphicsResult::Success);

        auto fragmentShader = std::move(createdFragmentShader);

        ci.vertexProgram = vertexShader;
        ci.fragmentProgram = fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<GBufferPassResources>> GBufferRenderer::PrepareGBufferPassResources()
    {
        UniquePtr<GBufferPassResources> gbufferResources = MakeUnique<GBufferPassResources>();

        auto[createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();

        if (createCommandPoolResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        gbufferResources->commandPool = std::move(createdCommandPool);

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
            LUCH_ASSERT(false);
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
            LUCH_ASSERT(false);
            return { false };
        }

        gbufferResources->meshBufferDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);

        auto[createMaterialTextureDescriptorSetLayoutResult, createdMaterialTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            materialTextureDescriptorSetLayoutCreateInfo);

        if (createMaterialTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        gbufferResources->materialTextureDescriptorSetLayout = std::move(createdMaterialTextureDescriptorSetLayout);

        auto[createMaterialBufferDescriptorSetLayoutResult, createdMaterialBufferDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            materialBufferDescriptorSetLayoutCreateInfo);

        if (createMaterialBufferDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        gbufferResources->materialBufferDescriptorSetLayout = std::move(createdMaterialBufferDescriptorSetLayout);

        auto[createMaterialSamplerDescriptorSetLayoutResult, createdMaterialSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            materialSamplerDescriptorSetLayoutCreateInfo);

        if (createMaterialSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        gbufferResources->materialSamplerDescriptorSetLayout = std::move(createdMaterialSamplerDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Vertex, commonResources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Vertex, gbufferResources->meshBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(
            pipelineLayoutCreateInfo);

        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
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

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedUniformBufferSize;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        gbufferResources->sharedBuffer = MakeUnique<SharedBuffer>(std::move(createdBuffer));

        return { true, std::move(gbufferResources) };
    }

    ResultValue<bool, UniquePtr<GBufferTextures>> GBufferRenderer::CreateGBufferTextures()
    {
        UniquePtr<GBufferTextures> textures = MakeUnique<GBufferTextures>();

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

        auto[createBaseColorTextureResult, createdBaseColorTexture] = context->device->CreateTexture(
            baseColorTextureCreateInfo);

        if (createBaseColorTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures->baseColorTexture = std::move(createdBaseColorTexture);

        // Normal map

        TextureCreateInfo normalMapCreateInfo;
        normalMapCreateInfo.format = normalMapFormat;
        normalMapCreateInfo.width = textureWidth;
        normalMapCreateInfo.height = textureHeight;
        normalMapCreateInfo.usage =
              TextureUsageFlags::ColorAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;


        auto[createNormapMapTextureResult, createdNormapMapTexture] = context->device->CreateTexture(
            normalMapCreateInfo);

        if (createNormapMapTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures->normalMapTexture = std::move(createdNormapMapTexture);

        TextureCreateInfo depthBufferCreateInfo;
        depthBufferCreateInfo.format = depthStencilFormat;
        depthBufferCreateInfo.width = textureWidth;
        depthBufferCreateInfo.height = textureHeight;
        depthBufferCreateInfo.usage =
              TextureUsageFlags::DepthStencilAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = context->device->CreateTexture(
            depthBufferCreateInfo);

        if (createDepthStencilBufferResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures->depthStencilBuffer = std::move(createdDepthStencilBuffer);

        return { true, std::move(textures) };
    }
}
