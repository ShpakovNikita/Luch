#include <Luch/Render/Passes/Forward/ForwardRenderPass.h>
#include <Luch/Render/Passes/Forward/ForwardContext.h>
#include <Luch/Render/TextureUploader.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
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

namespace Luch::Render::Passes::Forward
{
    using namespace Graphics;
    using namespace Graph;

    const String ForwardRenderPass::RenderPassName{ "Forward" };
    const String ForwardRenderPass::RenderPassNameWithDepthOnly{ "ForwardDepthOnly" };

    ForwardRenderPass::ForwardRenderPass(
        ForwardPersistentContext* aPersistentContext,
        ForwardTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        if(transientContext->useDepthPrepass)
        {
            auto node = builder->AddGraphicsRenderPass(RenderPassNameWithDepthOnly, persistentContext->renderPassWithDepthOnly, this);
            luminanceTextureHandle = node->CreateColorAttachment(0, { transientContext->outputSize });
            depthStencilTextureHandle = node->UseDepthStencilAttachment(transientContext->depthStencilTextureHandle);
        }
        else
        {
            auto node = builder->AddGraphicsRenderPass(RenderPassName, persistentContext->renderPass, this);
            luminanceTextureHandle = node->CreateColorAttachment(0, { transientContext->outputSize });
            depthStencilTextureHandle = node->CreateDepthStencilAttachment({ transientContext->outputSize });
        }
    }

    ForwardRenderPass::~ForwardRenderPass() = default;

    void ForwardRenderPass::PrepareScene()
    {
        const auto& nodes = transientContext->scene->GetNodes();

        for (const auto& node : nodes)
        {
            if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node);
            }
            if(node->GetCamera() != nullptr)
            {
                PrepareCameraNode(node);
            }
        }
    }

    void ForwardRenderPass::UpdateScene()
    {
        for (const auto& node : transientContext->scene->GetNodes())
        {
            UpdateNode(node);
        }
        
        const auto& sceneProperties = transientContext->scene->GetSceneProperties();

        RefPtrVector<SceneV1::Node> lightNodes(sceneProperties.lightNodes.begin(), sceneProperties.lightNodes.end());

        UpdateLights(lightNodes);
    }

    void ForwardRenderPass::ExecuteGraphicsRenderPass(
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

        DrawScene(transientContext->scene, commandList);
    }

    void ForwardRenderPass::PrepareNode(SceneV1::Node* node)
    {
        if (node->GetMesh() != nullptr)
        {
            PrepareMeshNode(node);
        }

        if(node->GetCamera() != nullptr)
        {
            PrepareCameraNode(node);
        }

        for (const auto& child : node->GetChildren())
        {
            PrepareNode(child);
        }
    }

    void ForwardRenderPass::PrepareMeshNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }
    }

    void ForwardRenderPass::PrepareCameraNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }
    }

    void ForwardRenderPass::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        const auto& renderPassName = GetRenderPassName(transientContext->useDepthPrepass);
        RefPtr<GraphicsPipelineState> pipelineState = primitive->GetGraphicsPipelineState(renderPassName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreatePipelineState(primitive, transientContext->useDepthPrepass, persistentContext);
            primitive->SetGraphicsPipelineState(renderPassName, pipelineState);
        }
    }

    void ForwardRenderPass::PrepareMesh(SceneV1::Mesh* mesh)
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

    void ForwardRenderPass::UpdateNode(SceneV1::Node* node)
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

    void ForwardRenderPass::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
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

    void ForwardRenderPass::UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes)
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
            persistentContext->lightingParamsBinding,
            lightingParamsSuballocation.buffer,
            lightingParamsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->lightsBufferBinding,
            lightsSuballocation.buffer,
            lightsSuballocation.offset);

        transientContext->lightsBufferDescriptorSet->Update();
    }

    void ForwardRenderPass::DrawScene(SceneV1::Scene* scene, GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            persistentContext->pipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        commandList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        commandList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->lightsBufferDescriptorSet);

        for (const auto& node : scene->GetNodes())
        {
            DrawNode(node, commandList);
        }
    }

    void ForwardRenderPass::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
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

    void ForwardRenderPass::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            persistentContext->pipelineLayout,
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

    void ForwardRenderPass::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
    {
        commandList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            material->GetTextureDescriptorSet());

        commandList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            material->GetBufferDescriptorSet());

        commandList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            material->GetSamplerDescriptorSet());
    }

    void ForwardRenderPass::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
    {
        auto& pipelineState = primitive->GetGraphicsPipelineState(GetRenderPassName(transientContext->useDepthPrepass));

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

    const String& ForwardRenderPass::GetRenderPassName(bool useDepthPrepass)
    {
        return useDepthPrepass ? RenderPassNameWithDepthOnly : RenderPassName;
    }

    RefPtr<GraphicsPipelineState> ForwardRenderPass::CreatePipelineState(
        SceneV1::Primitive* primitive,
        bool useDepthPrepass,
        ForwardPersistentContext* context)
    {
        GraphicsPipelineStateCreateInfo ci;

        ci.name = GetRenderPassName(useDepthPrepass);

        ShaderDefines shaderDefines;

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        LUCH_ASSERT(vertexBuffers.size() == 1);

        ci.inputAssembler.bindings.resize(vertexBuffers.size());
        for (int32 i = 0; i < vertexBuffers.size(); i++)
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

        ci.depthStencil.depthTestEnable = true;
        if(useDepthPrepass)
        {
            ci.depthStencil.depthWriteEnable = false;
            ci.depthStencil.depthCompareFunction = CompareFunction::Equal;
        }
        else
        {
            ci.depthStencil.depthWriteEnable = true;
            ci.depthStencil.depthCompareFunction = CompareFunction::Less;
        }

        ci.colorAttachments.attachments.resize(1);
        ci.colorAttachments.attachments[0].format = LuminanceFormat;

        ci.renderPass = context->renderPass;
        ci.pipelineLayout = context->pipelineLayout;

        if (material->HasBaseColorTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasBaseColorTexture);
        }

        if (material->HasMetallicRoughnessTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasMetallicRoughnessTexture);
        }

        if (material->HasNormalTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasNormalTexture);
        }

        if (material->HasOcclusionTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasOcclusionTexture);
        }

        if (material->HasEmissiveTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasEmissiveTexture);
        }

        if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Mask)
        {
            ci.name += " (Alphatest)";
            shaderDefines.AddFlag(MaterialShaderDefines::AlphaMask);
        }

        LUCH_ASSERT(material->GetProperties().alphaMode != SceneV1::AlphaMode::Blend);

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
            "Data/Shaders/",
            "Data/Shaders/Forward/",
            "forward_vp",
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
            "Data/Shaders/Forward/",
            "forward_fp",
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

    ResultValue<bool, UniquePtr<ForwardPersistentContext>> ForwardRenderPass::PrepareForwardPersistentContext(
        GraphicsDevice* device,
        CameraResources* cameraResources,
        MaterialResources* materialResources)
    {
        auto context = MakeUnique<ForwardPersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;
        context->materialResources = materialResources;

        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        {
            ColorAttachment luminanceColorAttachment;
            luminanceColorAttachment.format = LuminanceFormat;
            luminanceColorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
            luminanceColorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;
            luminanceColorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

            DepthStencilAttachment depthStencilAttachmentTemplate;
            depthStencilAttachmentTemplate.format = depthStencilFormat;
            depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
            depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::DontCare;
            depthStencilAttachmentTemplate.stencilLoadOperation = AttachmentLoadOperation::Clear;
            depthStencilAttachmentTemplate.stencilStoreOperation = AttachmentStoreOperation::DontCare;

            // Render Pass without Depth Prepass
            {
                RenderPassCreateInfo renderPassCreateInfo;
                renderPassCreateInfo.colorAttachments[0] = luminanceColorAttachment;
                renderPassCreateInfo.depthStencilAttachment = depthStencilAttachmentTemplate;
                renderPassCreateInfo.depthStencilAttachment->depthLoadOperation = AttachmentLoadOperation::Clear;
                renderPassCreateInfo.depthStencilAttachment->depthStoreOperation = AttachmentStoreOperation::Store;
                renderPassCreateInfo.depthStencilAttachment->stencilLoadOperation = AttachmentLoadOperation::Clear;
                renderPassCreateInfo.depthStencilAttachment->stencilStoreOperation = AttachmentStoreOperation::Store;
                renderPassCreateInfo.depthStencilAttachment->depthClearValue = 1.0;
                renderPassCreateInfo.depthStencilAttachment->stencilClearValue = 0x00000000;

                auto [createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
                if(createRenderPassResult != GraphicsResult::Success)
                {
                    return { false };
                }

                context->renderPass = std::move(createdRenderPass);
            }

            // Render Pass with Depth Prepass
            {
                RenderPassCreateInfo renderPassCreateInfo;
                renderPassCreateInfo.colorAttachments[0] = luminanceColorAttachment;
                renderPassCreateInfo.depthStencilAttachment = depthStencilAttachmentTemplate;
                renderPassCreateInfo.depthStencilAttachment->depthLoadOperation = AttachmentLoadOperation::Load;
                renderPassCreateInfo.depthStencilAttachment->depthStoreOperation = AttachmentStoreOperation::Store;
                renderPassCreateInfo.depthStencilAttachment->stencilLoadOperation = AttachmentLoadOperation::Load;
                renderPassCreateInfo.depthStencilAttachment->stencilStoreOperation = AttachmentStoreOperation::Store;

                auto [createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
                if(createRenderPassResult != GraphicsResult::Success)
                {
                    return { false };
                }

                context->renderPassWithDepthOnly = std::move(createdRenderPass);
            }
        }

        {
            DescriptorPoolCreateInfo descriptorPoolCreateInfo;
            descriptorPoolCreateInfo.maxDescriptorSets = MaxDescriptorSetCount;
            descriptorPoolCreateInfo.descriptorCount =
            {
                { ResourceType::Texture, MaxDescriptorCount },
                { ResourceType::UniformBuffer, MaxDescriptorCount },
                { ResourceType::Sampler, MaxDescriptorCount },
            };

            auto[createDescriptorPoolResult, createdDescriptorPool] = device->CreateDescriptorPool(
                descriptorPoolCreateInfo);

            if (createDescriptorPoolResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->descriptorPool = std::move(createdDescriptorPool);
        }

        context->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        {
            DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
            meshDescriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Buffer)
                .AddBinding(&context->meshUniformBufferBinding);

            auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = device->CreateDescriptorSetLayout(
                meshDescriptorSetLayoutCreateInfo);

            if (createMeshDescriptorSetLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->meshBufferDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);
        }

        context->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        context->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

        {
            DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
            lightsDescriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Buffer)
                .WithNBindings(2)
                .AddBinding(&context->lightingParamsBinding)
                .AddBinding(&context->lightsBufferBinding);

            auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
            if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo
                .AddSetLayout(ShaderStage::Vertex, cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Vertex, context->meshBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, materialResources->materialTextureDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, materialResources->materialBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, materialResources->materialSamplerDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->lightsBufferDescriptorSetLayout);

            auto[createPipelineLayoutResult, createdPipelineLayout] = device->CreatePipelineLayout(
                pipelineLayoutCreateInfo);

            if (createPipelineLayoutResult != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->pipelineLayout = std::move(createdPipelineLayout);
        }

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<ForwardTransientContext>> ForwardRenderPass::PrepareForwardTransientContext(
        ForwardPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<ForwardTransientContext>();
        context->descriptorPool = descriptorPool;

        auto [allocateLightsDescriptorSetResult, allocatedLightsBufferSet] = context->descriptorPool->AllocateDescriptorSet(
            persistentContext->lightsBufferDescriptorSetLayout);

        if(allocateLightsDescriptorSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->lightsBufferDescriptorSet = allocatedLightsBufferSet;

        return { true, std::move(context) };
    }
}
