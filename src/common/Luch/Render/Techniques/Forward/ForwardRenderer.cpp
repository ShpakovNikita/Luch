#include <Luch/Render/Techniques/Forward/ForwardRenderer.h>
#include <Luch/Render/Techniques/Forward/ForwardRendererContext.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/IndirectLightingResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/LightResources.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>

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
#include <Luch/Graphics/RenderPass.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>

namespace Luch::Render::Techniques::Forward
{
    using namespace Graphics;

    UniquePtr<ForwardRendererPersistentContext> ForwardRenderer::PrepareForwardRendererPersistentContext(
        const ForwardRendererPersistentContextCreateInfo& createInfo)
    {
        auto context = MakeUnique<ForwardRendererPersistentContext>();
        context->device = createInfo.device;
        context->cameraResources = createInfo.cameraResources;
        context->materialResources = createInfo.materialResources;
        context->indirectLightingResources = createInfo.indirectLightingResources;
        context->lightResources = createInfo.lightResources;
        context->renderPass = createInfo.renderPass;
        context->renderPassWithDepthPrepass = createInfo.renderPassWithDepthPrepass;

        {
            context->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

            DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
            descriptorSetLayoutCreateInfo
                .OfType(DescriptorSetType::Buffer)
                .WithNBindings(1)
                .AddBinding(&context->meshUniformBufferBinding);

            auto [result, createdDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);

            if (result != GraphicsResult::Success)
            {
                return nullptr;
            }

            context->meshBufferDescriptorSetLayout = std::move(createdDescriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
            pipelineLayoutCreateInfo
                .AddSetLayout(ShaderStage::Vertex, context->cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Vertex, context->meshBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->materialResources->materialTextureDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->materialResources->materialBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->materialResources->materialSamplerDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->lightResources->lightsBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(
                pipelineLayoutCreateInfo);

            if (createPipelineLayoutResult != GraphicsResult::Success)
            {
                return nullptr;
            }

            context->pipelineLayout = std::move(createdPipelineLayout);
        }

        return context;
    }

    UniquePtr<ForwardRendererTransientContext> ForwardRenderer::PrepareForwardRendererTransientContext(
        ForwardRendererPersistentContext* persistentContext,
        const ForwardRendererTransientContextCreateInfo& createInfo)
    {
        auto context = MakeUnique<ForwardRendererTransientContext>();
        context->descriptorPool = createInfo.descriptorPool;
        context->cameraBufferDescriptorSet = createInfo.cameraBufferDescriptorSet;
        context->sharedBuffer = createInfo.sharedBuffer;
        context->useDepthPrepass = createInfo.useDepthPrepass;
 
        {
            auto [result, allocatedBufferSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->lightResources->lightsBufferDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return nullptr;
            }

            context->lightsBufferDescriptorSet = allocatedBufferSet;
        }

        {
            auto [result, allocatedBufferSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return nullptr;
            }

            context->indirectLightingTexturesDescriptorSet = allocatedBufferSet;
        }

        return context;
    }

    ForwardRenderer::ForwardRenderer(
        ForwardRendererPersistentContext* aPersistentContext,
        ForwardRendererTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
    }

    void ForwardRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            PrepareNode(node);
        }
    }

    void ForwardRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        for (const auto& node : scene->GetNodes())
        {
            UpdateNode(node);
        }
        
        const auto& sceneProperties = scene->GetSceneProperties();

        RefPtrVector<SceneV1::Node> lightNodes(sceneProperties.lightNodes.begin(), sceneProperties.lightNodes.end());

        UpdateLights(lightNodes);
    }

    void ForwardRenderer::PrepareNode(SceneV1::Node* node)
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

    void ForwardRenderer::PrepareMeshNode(SceneV1::Node* node)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }
    }

    void ForwardRenderer::PreparePrimitive(SceneV1::Primitive* primitive)
    {
        const auto& pipelineStateName = GetPipelineStateName();;
        RefPtr<GraphicsPipelineState> pipelineState = primitive->GetGraphicsPipelineState(pipelineStateName);
        if (pipelineState == nullptr)
        {
            pipelineState = CreatePipelineState(primitive, transientContext->useDepthPrepass, persistentContext);
            primitive->SetGraphicsPipelineState(pipelineStateName, pipelineState);
        }
    }

    void ForwardRenderer::PrepareMesh(SceneV1::Mesh* mesh)
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

    void ForwardRenderer::UpdateNode(SceneV1::Node* node)
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

    void ForwardRenderer::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
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

    void ForwardRenderer::UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes)
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

        int32 enabledLightsCount = lightUniforms.size();

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

    void ForwardRenderer::UpdateIndirectLightingDescriptorSet(
        const IBLTextures& iblTextures,
        DescriptorSet* descriptorSet)
    {
        descriptorSet->WriteTexture(
            persistentContext->indirectLightingResources->diffuseIlluminanceCubemapBinding,
            iblTextures.diffuseIlluminanceCubemap);

        descriptorSet->WriteTexture(
            persistentContext->indirectLightingResources->specularReflectionCubemapBinding,
            iblTextures.specularReflectionCubemap);

        descriptorSet->WriteTexture(
            persistentContext->indirectLightingResources->specularBRDFTextureBinding,
            iblTextures.specularBRDFTexture);

        descriptorSet->Update();
    }

    void ForwardRenderer::DrawScene(
        SceneV1::Scene* scene,
        const IBLTextures& iblTextures,
        GraphicsCommandList* commandList)
    {
        UpdateIndirectLightingDescriptorSet(iblTextures, transientContext->indirectLightingTexturesDescriptorSet);

        commandList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->indirectLightingTexturesDescriptorSet);

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

    void ForwardRenderer::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
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

    void ForwardRenderer::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
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

    void ForwardRenderer::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
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

    void ForwardRenderer::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
    {
        auto& pipelineState = primitive->GetGraphicsPipelineState(GetPipelineStateName());

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

    const String& ForwardRenderer::GetPipelineStateName()
    {
        RenderPass* renderPass = transientContext->useDepthPrepass ? persistentContext->renderPass : persistentContext->renderPassWithDepthPrepass;
        return renderPass->GetCreateInfo().name;
    }

    RefPtr<GraphicsPipelineState> ForwardRenderer::CreatePipelineState(
        SceneV1::Primitive* primitive,
        bool useDepthPrepass,
        ForwardRendererPersistentContext* context)
    {
        GraphicsPipelineStateCreateInfo ci;

        ci.name = useDepthPrepass ? "Forward_DepthPrepass" : "Forward";

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
        ci.colorAttachments.attachments[0].format = context->renderPass->GetCreateInfo().colorAttachments.front()->format;

        ci.renderPass = useDepthPrepass ? context->renderPassWithDepthPrepass : context->renderPass;
        ci.pipelineLayout = context->pipelineLayout;

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
            "Data/Shaders/Forward/",
            "forward_vp",
            shaderDefines.defines);

        if (!vertexShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
        }

        auto[vertexShaderCreateResult, createdVertexShader] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "VertexMain");
        LUCH_ASSERT(vertexShaderCreateResult == GraphicsResult::Success);

        auto vertexShader = std::move(createdVertexShader);

        auto[fragmentShaderLibraryCreated, fragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
            "Data/Shaders/",
            "Data/Shaders/Forward/",
            material->GetProperties().unlit ? "forward_unlit_fp" : "forward_fp",
            shaderDefines.defines);

        if (!fragmentShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
        }

        auto[fragmentShaderCreateResult, createdFragmentShader] = fragmentShaderLibrary->CreateShaderProgram(
            ShaderStage::Fragment,
            "FragmentMain");
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
}
