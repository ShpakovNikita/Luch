#include <Luch/Render/Passes/Deferred/GBufferRenderPass.h>
#include <Luch/Render/TextureUploader.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/Passes/Deferred/GBufferContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>

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
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>

namespace Luch::Render::Passes::Deferred
{
    using namespace Graphics;
    using namespace Graph;

    const String GBufferRenderPass::RenderPassWithDepthOnlyName{ "GBufferWithDepthOnly" };
    const String GBufferRenderPass::RenderPassName{ "GBuffer" };

    GBufferRenderPass::GBufferRenderPass(
        GBufferPersistentContext* aPersistentContext,
        GBufferTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        UniquePtr<RenderGraphNodeBuilder> node;

        if(transientContext->useDepthPrepass)
        {
            node = builder->AddGraphicsPass(RenderPassWithDepthOnlyName, persistentContext->renderPassWithDepthOnly, this);
            gbuffer.depthStencil = node->UseDepthStencilAttachment(transientContext->depthStencilTextureHandle);
        }
        else
        {
            node = builder->AddGraphicsPass(RenderPassName, persistentContext->renderPass, this);
            gbuffer.depthStencil = node->CreateDepthStencilAttachment({ transientContext->outputSize });
        }

        for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
        {
            gbuffer.color[i] = node->CreateColorAttachment(i, { transientContext->outputSize });
        }
    }

    GBufferRenderPass::~GBufferRenderPass() = default;

    void GBufferRenderPass::PrepareScene()
    {
        const auto& nodes = transientContext->scene->GetNodes();

        for (const auto& node : nodes)
        {
            if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node);
            }
        }
    }

    void GBufferRenderPass::UpdateScene()
    {
        for (const auto& node : transientContext->scene->GetNodes())
        {
            UpdateNode(node);
        }
    }

    void GBufferRenderPass::ExecuteGraphicsPass(
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
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            persistentContext->pipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        for (const auto& node : transientContext->scene->GetNodes())
        {
            DrawNode(node, commandList);
        }
    }

    void GBufferRenderPass::PrepareNode(SceneV1::Node* node)
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

    void GBufferRenderPass::PrepareMeshNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }
    }

    void GBufferRenderPass::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        const auto& passName = GetRenderPassName(transientContext->useDepthPrepass);
        RefPtr<GraphicsPipelineState> pipelineState = primitive->GetGraphicsPipelineState(passName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreateGBufferPipelineState(primitive, transientContext->useDepthPrepass);
            primitive->SetGraphicsPipelineState(passName, pipelineState);
        }
    }

    void GBufferRenderPass::PrepareMesh(SceneV1::Mesh* mesh)
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

    void GBufferRenderPass::UpdateNode(SceneV1::Node* node)
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

    void GBufferRenderPass::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
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

    void GBufferRenderPass::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
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

    void GBufferRenderPass::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
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

    void GBufferRenderPass::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
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

    void GBufferRenderPass::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
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

    const String& GBufferRenderPass::GetRenderPassName(bool useDepthPrepass)
    {
        return useDepthPrepass ? RenderPassWithDepthOnlyName : RenderPassName; 
    }

    RefPtr<GraphicsPipelineState> GBufferRenderPass::CreateGBufferPipelineState(
        SceneV1::Primitive* primitive,
        bool useDepthPrepass)
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

        if(useDepthPrepass)
        {
            ci.depthStencil.depthTestEnable = true;
            ci.depthStencil.depthWriteEnable = false;
            ci.depthStencil.depthCompareFunction = CompareFunction::Equal;
        }
        else
        {
            ci.depthStencil.depthTestEnable = true;
            ci.depthStencil.depthWriteEnable = true;
            ci.depthStencil.depthCompareFunction = CompareFunction::Less;
        }

        ci.colorAttachments.attachments.resize(DeferredConstants::GBufferColorAttachmentCount);
        for(int32 i = 0; i < ci.colorAttachments.attachments.size(); i++)
        {
            ci.colorAttachments.attachments[i].format = DeferredConstants::GBufferColorAttachmentFormats[i];
        }

        ci.renderPass = persistentContext->renderPass;
        ci.pipelineLayout = persistentContext->pipelineLayout;

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
            persistentContext->device,
            "Data/Shaders/",
            "Data/Shaders/Deferred/",
            "gbuffer_vp",
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
            persistentContext->device,
            "Data/Shaders/",
            "Data/Shaders/Deferred/",
            "gbuffer_fp",
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

        auto[createPipelineResult, createdPipeline] = persistentContext->device->CreateGraphicsPipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<GBufferPersistentContext>> GBufferRenderPass::PrepareGBufferPersistentContext(
        GraphicsDevice* device,
        CameraResources* cameraResources,
        MaterialResources* materialResources)
    {
        UniquePtr<GBufferPersistentContext> context = MakeUnique<GBufferPersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;
        context->materialResources = materialResources;

        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        // Render pass for gbuffer
        ColorAttachment gbufferColorAttachmentTemplate;
        gbufferColorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        gbufferColorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
        gbufferColorAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };

        DepthStencilAttachment depthStencilAttachment;
        depthStencilAttachment.format = depthStencilFormat;
        depthStencilAttachment.depthLoadOperation = AttachmentLoadOperation::Clear;
        depthStencilAttachment.depthStoreOperation = AttachmentStoreOperation::Store;
        depthStencilAttachment.stencilLoadOperation = AttachmentLoadOperation::Clear;
        depthStencilAttachment.stencilStoreOperation = AttachmentStoreOperation::Store;
        depthStencilAttachment.depthClearValue = 1.0;
        depthStencilAttachment.stencilClearValue = 0x00000000;

        RenderPassCreateInfo renderPassCreateInfo;
        for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
        {
            ColorAttachment attachment = gbufferColorAttachmentTemplate;
            attachment.format = DeferredConstants::GBufferColorAttachmentFormats[i];
            renderPassCreateInfo.colorAttachments[i] = attachment;
        }
        renderPassCreateInfo.depthStencilAttachment = depthStencilAttachment;

        auto [createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
        if(createRenderPassResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->renderPass = std::move(createdRenderPass);

        // Render pass for gbuffer with depth only
        renderPassCreateInfo.depthStencilAttachment->depthLoadOperation = AttachmentLoadOperation::Load;
        renderPassCreateInfo.depthStencilAttachment->stencilLoadOperation = AttachmentLoadOperation::Load;

        auto [createRenderPassWithDepthOnlyResult, createdRenderPassWithDepthOnly] = device->CreateRenderPass(renderPassCreateInfo);
        if(createRenderPassWithDepthOnlyResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->renderPassWithDepthOnly = std::move(createdRenderPassWithDepthOnly);

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

        context->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

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

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Vertex, cameraResources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Vertex, context->meshBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, materialResources->materialTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, materialResources->materialBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, materialResources->materialSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = device->CreatePipelineLayout(
            pipelineLayoutCreateInfo);

        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->pipelineLayout = std::move(createdPipelineLayout);

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<GBufferTransientContext>> GBufferRenderPass::PrepareGBufferTransientContext(
        GBufferPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<GBufferTransientContext>();
        context->descriptorPool = descriptorPool;
        return { true, std::move(context) };
    }
}
