#include <Luch/Render/Passes/DepthOnlyRenderPass.h>
#include <Luch/Render/TextureUploader.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Passes/DepthOnlyContext.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
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
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/PhysicalDeviceCapabilities.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/GraphicsPipelineState.h>
#include <Luch/Graphics/GraphicsPipelineStateCreateInfo.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>

namespace Luch::Render::Passes
{
    using namespace Graphics;
    using namespace Graph;

    const String DepthOnlyRenderPass::RenderPassName{"DepthOnly"};

    DepthOnlyRenderPass::DepthOnlyRenderPass(
        DepthOnlyPersistentContext* aPersistentContext,
        DepthOnlyTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        auto node = builder->AddGraphicsRenderPass(RenderPassName, persistentContext->renderPass, this);

        depthTextureHandle = node->CreateDepthStencilAttachment({ transientContext->outputSize });
    }

    DepthOnlyRenderPass::~DepthOnlyRenderPass() = default;

    void DepthOnlyRenderPass::PrepareScene()
    {
        const auto& nodes = transientContext->scene->GetNodes();

        for (const auto& node : nodes)
        {
            PrepareNode(node);
        }
    }

    void DepthOnlyRenderPass::UpdateScene()
    {
        for (const auto& node : transientContext->scene->GetNodes())
        {
            UpdateNode(node);
        }
    }

    void DepthOnlyRenderPass::ExecuteGraphicsRenderPass(
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

    void DepthOnlyRenderPass::PrepareNode(SceneV1::Node* node)
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

    void DepthOnlyRenderPass::PrepareMeshNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }
    }

    void DepthOnlyRenderPass::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        RefPtr<GraphicsPipelineState> pipelineState = primitive->GetGraphicsPipelineState(RenderPassName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreateDepthOnlyPipelineState(primitive);
            primitive->SetGraphicsPipelineState(RenderPassName, pipelineState);
        }
    }

    void DepthOnlyRenderPass::PrepareMesh(SceneV1::Mesh* mesh)
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

    void DepthOnlyRenderPass::UpdateNode(SceneV1::Node* node)
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

    void DepthOnlyRenderPass::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
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

    void DepthOnlyRenderPass::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
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

    void DepthOnlyRenderPass::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
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

    void DepthOnlyRenderPass::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
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

    void DepthOnlyRenderPass::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
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

    RefPtr<GraphicsPipelineState> DepthOnlyRenderPass::CreateDepthOnlyPipelineState(SceneV1::Primitive* primitive)
    {
        GraphicsPipelineStateCreateInfo ci;

        ci.name = "DepthOnly";

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

        bool hasTexcoord = false;
        const auto& attributes = primitive->GetAttributes();
        ci.inputAssembler.attributes.resize(SemanticToLocation.size());
        for (const auto& attribute : attributes)
        {
            auto& attributeDescription = ci.inputAssembler.attributes[SemanticToLocation.at(attribute.semantic)];
            attributeDescription.binding = attribute.vertexBufferIndex;
            attributeDescription.format = attribute.format;
            attributeDescription.offset = attribute.offset;

            shaderDefines.AddFlag(SemanticToFlag.at(attribute.semantic));

            if(attribute.semantic == SceneV1::AttributeSemantic::Texcoord_0)
            {
                hasTexcoord = true;
            }
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
        ci.depthStencil.depthWriteEnable = true;
        ci.depthStencil.depthCompareFunction = CompareFunction::Less;

        ci.renderPass = persistentContext->renderPass;
        ci.pipelineLayout = persistentContext->pipelineLayout;

        if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Mask)
        {
            ci.name += " (Alphatest)";
            LUCH_ASSERT_MSG(hasTexcoord, "Can't alpha-test without texcoords");
            LUCH_ASSERT_MSG(material->HasBaseColorTexture(), "Can't alpha-test without base color");
            shaderDefines.AddFlag(MaterialShaderDefines::AlphaMask);
            shaderDefines.AddFlag(MaterialShaderDefines::HasBaseColorTexture);
        }

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            persistentContext->device,
            "Data/Shaders/",
            "Data/Shaders/",
            "depth_only_vp",
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

        ci.vertexProgram = vertexShader;

        if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Mask)
        {
            auto[fragmentShaderLibraryCreated, fragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
                persistentContext->device,
                "Data/Shaders/",
                "Data/Shaders/",
                "depth_only_fp",
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

            ci.fragmentProgram = fragmentShader;
        }

        auto[createPipelineResult, createdPipeline] = persistentContext->device->CreateGraphicsPipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<DepthOnlyPersistentContext>> DepthOnlyRenderPass::PrepareDepthOnlyPersistentContext(
        GraphicsDevice* device,
        CameraResources* cameraResources,
        MaterialResources* materialResources)
    {
        auto context = MakeUnique<DepthOnlyPersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;
        context->materialResources = materialResources;

        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        DepthStencilAttachment depthStencilAttachment;
        depthStencilAttachment.format = depthStencilFormat;
        depthStencilAttachment.depthLoadOperation = AttachmentLoadOperation::Clear;
        depthStencilAttachment.depthStoreOperation = AttachmentStoreOperation::Store;
        depthStencilAttachment.stencilLoadOperation = AttachmentLoadOperation::Clear;
        depthStencilAttachment.stencilStoreOperation = AttachmentStoreOperation::Store;
        depthStencilAttachment.depthClearValue = 1.0;
        depthStencilAttachment.stencilClearValue = 0x00000000;

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.depthStencilAttachment = depthStencilAttachment;

        auto [createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
        if(createRenderPassResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->renderPass = std::move(createdRenderPass);

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

    ResultValue<bool, UniquePtr<DepthOnlyTransientContext>> DepthOnlyRenderPass::PrepareDepthOnlyTransientContext(
        DepthOnlyPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<DepthOnlyTransientContext>();
        context->descriptorPool = descriptorPool;
        return { true, std::move(context) };
    }
}
