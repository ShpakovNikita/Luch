#include <Luch/Render/Passes/TiledDeferred/TiledDeferredRenderPass.h>
#include <Luch/Render/Passes/TiledDeferred/TiledDeferredContext.h>
#include <Luch/Render/Passes/TiledDeferred/TiledDeferredConstants.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/IndirectLightingResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/LightResources.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/AttributeSemantic.h>

#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/PhysicalDeviceCapabilities.h>
#include <Luch/Graphics/GraphicsPipelineState.h>
#include <Luch/Graphics/GraphicsPipelineStateCreateInfo.h>
#include <Luch/Graphics/TiledPipelineStateCreateInfo.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>

namespace Luch::Render::Passes::TiledDeferred
{
    using namespace Graphics;
    using namespace Graph;

    const String TiledDeferredRenderPass::RenderPassName{ "TiledDeferred" };

    TiledDeferredRenderPass::TiledDeferredRenderPass(
        TiledDeferredPersistentContext* aPersistentContext,
        TiledDeferredTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
    }

    TiledDeferredRenderPass::~TiledDeferredRenderPass() = default;

    void TiledDeferredRenderPass::PrepareScene()
    {
        const auto& nodes = transientContext->scene->GetNodes();

        for (const auto& node : nodes)
        {
            PrepareNode(node);
        }
    }

    void TiledDeferredRenderPass::UpdateScene()
    {
        for (const auto& node : transientContext->scene->GetNodes())
        {
            UpdateNode(node);
        }
        
        const auto& sceneProperties = transientContext->scene->GetSceneProperties();

        RefPtrVector<SceneV1::Node> lightNodes(sceneProperties.lightNodes.begin(), sceneProperties.lightNodes.end());

        UpdateLights(lightNodes);
    }

    void TiledDeferredRenderPass::Initialize(RenderGraphBuilder* builder)
    {
        auto node = builder->AddGraphicsPass(RenderPassName, persistentContext->renderPass, this);

        node->SetAttachmentSize(transientContext->outputSize);

        for(uint32 i = TiledDeferredConstants::GBufferColorAttachmentBegin; i < TiledDeferredConstants::GBufferColorAttachmentEnd; i++)
        {
            node->CreateColorAttachment(i, { ResourceStorageMode::Memoryless });
        }

        node->CreateDepthStencilAttachment({ ResourceStorageMode::Memoryless });

        luminanceTextureHandle = node->CreateColorAttachment(TiledDeferredConstants::LuminanceAttachmentIndex);

        if(transientContext->diffuseIlluminanceCubemapHandle)
        {
            diffuseIlluminanceCubemapHandle = node->ReadsTexture(transientContext->diffuseIlluminanceCubemapHandle);
        }

        if(transientContext->specularReflectionCubemapHandle && transientContext->specularBRDFTextureHandle)
        {
            specularReflectionCubemapHandle = node->ReadsTexture(transientContext->specularReflectionCubemapHandle);
            specularBRDFTextureHandle = node->ReadsTexture(transientContext->specularBRDFTextureHandle);
        }
    }

    void TiledDeferredRenderPass::ExecuteGraphicsPass(
        RenderGraphResourceManager* manager,
        GraphicsCommandList* commandList)
    {
        Viewport viewport;
        viewport.width = static_cast<float32>(transientContext->outputSize.width);
        viewport.height = static_cast<float32>(transientContext->outputSize.height);

        Rect2i scissorRect;
        scissorRect.size = transientContext->outputSize;

        commandList->SetViewports({ viewport });
        commandList->SetScissorRects({ scissorRect });

        DrawGBuffer(manager, commandList);
        Resolve(manager, commandList);
    }

    void TiledDeferredRenderPass::UpdateIndirectLightingDescriptorSet(
        RenderGraphResourceManager* manager,
        DescriptorSet* descriptorSet)
    {
        if(diffuseIlluminanceCubemapHandle)
        {
            auto diffuseIlluminanceCubemap = manager->GetTexture(diffuseIlluminanceCubemapHandle);

            transientContext->indirectLightingTexturesDescriptorSet->WriteTexture(
                persistentContext->indirectLightingResources->diffuseIlluminanceCubemapBinding,
                diffuseIlluminanceCubemap);
        }

        if(specularReflectionCubemapHandle && specularBRDFTextureHandle)
        {
            auto specularReflectionCubemap = manager->GetTexture(specularReflectionCubemapHandle);

            descriptorSet->WriteTexture(
                persistentContext->indirectLightingResources->specularReflectionCubemapBinding,
                specularReflectionCubemap);

            auto specularBRDFTexture = manager->GetTexture(specularBRDFTextureHandle);

            descriptorSet->WriteTexture(
                persistentContext->indirectLightingResources->specularBRDFTextureBinding,
                specularBRDFTexture);
        }

        descriptorSet->Update();
    }

    void TiledDeferredRenderPass::PrepareNode(SceneV1::Node* node)
    {
        if (node->GetMesh() != nullptr)
        {
            PrepareMeshNode(node);
        }

        for (const auto& child : node->GetChildren())
        {
            PrepareNode(child);
        }
    }

    void TiledDeferredRenderPass::PrepareMeshNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }
    }

    void TiledDeferredRenderPass::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        RefPtr<GraphicsPipelineState> pipelineState = primitive->GetGraphicsPipelineState(RenderPassName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreateGBufferPipelineState(primitive, persistentContext);
            primitive->SetGraphicsPipelineState(RenderPassName, pipelineState);
        }
    }

    void TiledDeferredRenderPass::PrepareMesh(SceneV1::Mesh* mesh)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            PreparePrimitive(primitive);
        }

        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = transientContext->descriptorPool->AllocateDescriptorSet(
            persistentContext->meshBufferDescriptorSetLayout);

        LUCH_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);

        meshDescriptorSets[mesh] = allocatedDescriptorSet;
    }

    void TiledDeferredRenderPass::UpdateNode(SceneV1::Node* node)
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

    void TiledDeferredRenderPass::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
    {
        MeshUniform meshUniform;
        meshUniform.transform = transform;
        meshUniform.inverseTransform = glm::inverse(transform);

        // TODO
        auto suballocation = transientContext->sharedBuffer->Suballocate(sizeof(MeshUniform), 256);

        memcpy(suballocation.offsetMemory, &meshUniform, sizeof(MeshUniform));

        auto& descriptorSet = meshDescriptorSets[mesh];

        descriptorSet->WriteUniformBuffer(
            persistentContext->meshUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        descriptorSet->Update();
    }

    void TiledDeferredRenderPass::UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes)
    {
        Vector<LightUniform> lightUniforms;

        for(const auto& lightNode : lightNodes)
        {
            const auto& light = lightNode->GetLight();
            LUCH_ASSERT(light != nullptr);

            if(light->IsEnabled())
            {
                LightUniform lightUniform = RenderUtils::GetLightUniform(light, lightNode->GetWorldTransform());
                lightUniforms.push_back(lightUniform);
            }
        }

        int32 enabledLightsCount = lightNodes.size();

        LightingParamsUniform lightingParams;
        lightingParams.lightCount = enabledLightsCount;

        auto lightingParamsSuballocation = transientContext->sharedBuffer->Suballocate(sizeof(LightingParamsUniform), 256);
        auto lightsSuballocation = transientContext->sharedBuffer->Suballocate(enabledLightsCount * sizeof(LightUniform), 256);

        memcpy(lightingParamsSuballocation.offsetMemory, &lightingParams, sizeof(LightingParamsUniform));
        memcpy(lightsSuballocation.offsetMemory, lightUniforms.data(), enabledLightsCount * sizeof(LightUniform));

        transientContext->lightsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->lightResources->lightingParamsBinding,
            lightingParamsSuballocation.buffer,
            lightingParamsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->lightResources->lightsBufferBinding,
            lightsSuballocation.buffer,
            lightsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->Update();
    }

    void TiledDeferredRenderPass::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
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

    void TiledDeferredRenderPass::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            persistentContext->gbufferPipelineLayout,
            meshDescriptorSets[mesh]);

        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->GetProperties().alphaMode != SceneV1::AlphaMode::Blend)
            {
                BindMaterial(material, commandList);
                DrawPrimitive(primitive, commandList);
            }
        }
    }

    void TiledDeferredRenderPass::DrawGBuffer(
        RenderGraphResourceManager*,
        GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            persistentContext->gbufferPipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        for (const auto& node : transientContext->scene->GetNodes())
        {
            DrawNode(node, commandList);
        }
    }

    void TiledDeferredRenderPass::Resolve(
        RenderGraphResourceManager* manager,
        GraphicsCommandList* commandList)
    {
        UpdateIndirectLightingDescriptorSet(manager, transientContext->indirectLightingTexturesDescriptorSet);

        commandList->BindTiledPipelineState(persistentContext->resolvePipelineState);

        commandList->BindBufferDescriptorSet(
            ShaderStage::Tile,
            persistentContext->resolvePipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        commandList->BindBufferDescriptorSet(
            ShaderStage::Tile,
            persistentContext->resolvePipelineLayout,
            transientContext->lightsBufferDescriptorSet);

        commandList->BindTextureDescriptorSet(
            ShaderStage::Tile,
            persistentContext->resolvePipelineLayout,
            transientContext->indirectLightingTexturesDescriptorSet);

        commandList->DispatchThreadsPerTile(commandList->GetTileSize());
    }

    void TiledDeferredRenderPass::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
    {
        commandList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->gbufferPipelineLayout,
            material->GetTextureDescriptorSet());

        commandList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->gbufferPipelineLayout,
            material->GetBufferDescriptorSet());

        commandList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->gbufferPipelineLayout,
            material->GetSamplerDescriptorSet());
    }

    void TiledDeferredRenderPass::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
    {
        auto& pipelineState = primitive->GetGraphicsPipelineState(RenderPassName);

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

        commandList->BindGraphicsPipelineState(pipelineState);
        commandList->BindVertexBuffers(graphicsVertexBuffers, offsets);

        commandList->BindIndexBuffer(
                indexBuffer.backingBuffer->GetDeviceBuffer(),
                indexBuffer.indexType,
                indexBuffer.byteOffset);

        commandList->DrawIndexedInstanced(indexBuffer.count, 0, 1, 0);
    }

    RefPtr<GraphicsPipelineState> TiledDeferredRenderPass::CreateGBufferPipelineState(
        SceneV1::Primitive* primitive,
        TiledDeferredPersistentContext* context)
    {
        GraphicsPipelineStateCreateInfo ci;

        ci.name = "GBuffer (Tiled)";

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        LUCH_ASSERT(vertexBuffers.size() == 1);

        ci.inputAssembler = RenderUtils::GetPrimitiveVertexInputStateCreateInfo(primitive);

        const auto& material = primitive->GetMaterial();

        if (material->GetProperties().doubleSided)
        {
            ci.rasterization.cullMode = CullMode::None;
        }
        else
        {
            ci.rasterization.cullMode = CullMode::Back;
        }

        ci.depthStencil.depthTestEnable = true;
        ci.depthStencil.depthWriteEnable = true;
        ci.depthStencil.depthCompareFunction = CompareFunction::Less;

        ci.colorAttachments.attachments.resize(TiledDeferredConstants::ColorAttachmentCount);
        for(uint32 i = 0; i < ci.colorAttachments.attachments.size(); i++)
        {
            ci.colorAttachments.attachments[i].format = TiledDeferredConstants::ColorAttachmentFormats[i];
        }

        ci.renderPass = context->renderPass;
        ci.pipelineLayout = context->gbufferPipelineLayout;

        ShaderDefines shaderDefines;
        RenderUtils::AddPrimitiveVertexShaderDefines(primitive, shaderDefines);
        RenderUtils::AddMaterialShaderDefines(material, shaderDefines);

        if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Mask)
        {
            ci.name += " (Alphatest)";
        }

        if (material->GetProperties().unlit)
        {
            ci.name += " (Unlit)";
        }

        LUCH_ASSERT(material->GetProperties().alphaMode != SceneV1::AlphaMode::Blend);

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
            "Data/Shaders/",
            "Data/Shaders/TiledDeferred/",
            "tiled_gbuffer_vp",
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
            "Data/Shaders/",
            "Data/Shaders/TiledDeferred/",
            material->GetProperties().unlit ? "tiled_gbuffer_unlit_fp" : "tiled_gbuffer_fp",
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

        auto[createPipelineResult, createdPipeline] = context->device->CreateGraphicsPipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    RefPtr<TiledPipelineState> TiledDeferredRenderPass::CreateResolvePipelineState(TiledDeferredPersistentContext* context)
    {
        TiledPipelineStateCreateInfo ci;

        ci.name = "Resolve (Tiled)";

        auto [tiledShaderLibraryCreated, createdTiledShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
            "Data/Shaders/",
            "Data/Shaders/TiledDeferred/",
            "tiled_resolve",
            {});

        if (!tiledShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
            return nullptr;
        }

        auto [tileShaderProgramCreateResult, tileShaderProgram] = createdTiledShaderLibrary->CreateShaderProgram(ShaderStage::Tile, "tile_main");
        if(tileShaderProgramCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return nullptr;
        }

        for(uint32 i = 0; i < TiledDeferredConstants::ColorAttachmentCount; i++)
        {
            auto& attachment = ci.colorAttachments.attachments.emplace_back();
            attachment.format = TiledDeferredConstants::ColorAttachmentFormats[i];
        }

        ci.tiledProgram = tileShaderProgram;
        ci.renderPass = context->renderPass;
        ci.pipelineLayout = context->resolvePipelineLayout;

        auto[createPipelineResult, createdPipeline] = context->device->CreateTiledPipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<TiledDeferredPersistentContext>> TiledDeferredRenderPass::PrepareTiledDeferredPersistentContext(
        GraphicsDevice* device,
        CameraPersistentResources* cameraResources,
        MaterialPersistentResources* materialResources,
        IndirectLightingPersistentResources* indirectLightingResources,
        LightPersistentResources* lightResources)
    {
        auto context = MakeUnique<TiledDeferredPersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;
        context->materialResources = materialResources;
        context->indirectLightingResources = indirectLightingResources;
        context->lightResources = lightResources;

        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        {
            ColorAttachment gbufferColorAttachmentTemplate;
            gbufferColorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
            gbufferColorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::DontCare;
            gbufferColorAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };

            ColorAttachment luminanceColorAttachment;
            luminanceColorAttachment.format = TiledDeferredConstants::ColorAttachmentFormats[TiledDeferredConstants::LuminanceAttachmentIndex];
            luminanceColorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
            luminanceColorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;
            luminanceColorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

            DepthStencilAttachment depthStencilAttachment;
            depthStencilAttachment.format = depthStencilFormat;
            depthStencilAttachment.depthLoadOperation = AttachmentLoadOperation::Clear;
            depthStencilAttachment.depthStoreOperation = AttachmentStoreOperation::DontCare;
            depthStencilAttachment.stencilLoadOperation = AttachmentLoadOperation::Clear;
            depthStencilAttachment.stencilStoreOperation = AttachmentStoreOperation::DontCare;
            depthStencilAttachment.depthClearValue = 1.0;
            depthStencilAttachment.stencilClearValue = 0x00000000;

            RenderPassCreateInfo createInfo;
            createInfo.name = RenderPassName;
            for(uint32 i = TiledDeferredConstants::GBufferColorAttachmentBegin; i < TiledDeferredConstants::GBufferColorAttachmentEnd; i++)
            {
                ColorAttachment attachment = gbufferColorAttachmentTemplate;
                attachment.format = TiledDeferredConstants::ColorAttachmentFormats[i];
                createInfo.colorAttachments[i] = attachment;
            }

            createInfo.colorAttachments[TiledDeferredConstants::LuminanceAttachmentIndex] = luminanceColorAttachment;
            createInfo.depthStencilAttachment = depthStencilAttachment;

            auto [result, renderPass] = device->CreateRenderPass(createInfo);
            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->renderPass = std::move(renderPass);
        }

        {
            DescriptorPoolCreateInfo createInfo;
            createInfo.maxDescriptorSets = MaxDescriptorSetCount;
            createInfo.descriptorCount =
            {
                { ResourceType::Texture, MaxDescriptorCount },
                { ResourceType::UniformBuffer, MaxDescriptorCount },
                { ResourceType::Sampler, MaxDescriptorCount },
            };

            auto [result, descriptorPool] = device->CreateDescriptorPool(createInfo);

            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->descriptorPool = std::move(descriptorPool);
        }

        {
            context->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .OfType(DescriptorSetType::Buffer)
                .AddBinding(&context->meshUniformBufferBinding);

            auto[result, meshDescriptorSetLayout] = device->CreateDescriptorSetLayout(createInfo);

            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->meshBufferDescriptorSetLayout = std::move(meshDescriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo createInfo;
            createInfo
                .AddSetLayout(ShaderStage::Vertex, cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Vertex, context->meshBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, materialResources->materialTextureDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, materialResources->materialBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, materialResources->materialSamplerDescriptorSetLayout);

            auto[result, gbufferPipelineLayout] = device->CreatePipelineLayout(createInfo);

            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->gbufferPipelineLayout = std::move(gbufferPipelineLayout);
        }

        {
            PipelineLayoutCreateInfo createInfo;
            createInfo
                .AddSetLayout(ShaderStage::Tile, context->cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Tile, context->lightResources->lightsBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Tile, context->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            auto [result, resolvePipelineLayout] = device->CreatePipelineLayout(createInfo);

            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->resolvePipelineLayout = std::move(resolvePipelineLayout);
        }

        context->resolvePipelineState = CreateResolvePipelineState(context.get());

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<TiledDeferredTransientContext>> TiledDeferredRenderPass::PrepareTiledDeferredTransientContext(
        TiledDeferredPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<TiledDeferredTransientContext>();
        context->descriptorPool = descriptorPool;

        {
            auto [result, lightsBufferSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->lightResources->lightsBufferDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->lightsBufferDescriptorSet = lightsBufferSet;
        }

        {
            auto [result, indirectLightingTextureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->indirectLightingTexturesDescriptorSet = indirectLightingTextureSet;
        }

        return { true, std::move(context) };
    }
}
