#include <Luch/Render/Passes/Deferred/ResolveRenderPass.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/IndirectLightingResources.h>
#include <Luch/Render/Passes/Deferred/ResolveContext.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/Light.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/FrameBuffer.h>
#include <Luch/Graphics/Texture.h>
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

    // One triangle that covers whole screen
    constexpr Array<QuadVertex, 3> fullscreenQuadVertices =
    {
        QuadVertex { Vec3{-1.0f, -1.0f, 0.0f}, Vec2{0.0f, +1.0f} },
        QuadVertex { Vec3{+3.0f, -1.0f, 0.0f}, Vec2{2.0f, +1.0f} },
        QuadVertex { Vec3{-1.0f, +3.0f, 0.0f}, Vec2{0.0f, -1.0f} },
    };

    const String ResolveRenderPass::RenderPassName{"Resolve"};

    ResolveRenderPass::ResolveRenderPass(
        ResolvePersistentContext* aPersistentContext,
        ResolveTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        auto node = builder->AddGraphicsPass(RenderPassName, persistentContext->renderPass, this);

        for(int32 i = 0; i < transientContext->gbuffer.color.size(); i++)
        {
            gbuffer.color[i] = node->ReadsTexture(transientContext->gbuffer.color[i]);
        }

        gbuffer.depthStencil = node->ReadsTexture(transientContext->gbuffer.depthStencil);

        if(transientContext->diffuseIlluminanceCubemapHandle)
        {
            diffuseIlluminanceCubemapHandle = node->ReadsTexture(transientContext->diffuseIlluminanceCubemapHandle);
        }

        if(transientContext->specularReflectionCubemapHandle && transientContext->specularBRDFTextureHandle)
        {
            specularReflectionCubemapHandle = node->ReadsTexture(transientContext->specularReflectionCubemapHandle);
            specularBRDFTextureHandle = node->ReadsTexture(transientContext->specularBRDFTextureHandle);
        }

        luminanceTextureHandle = node->CreateColorAttachment(0, { transientContext->outputSize });
    }

    ResolveRenderPass::~ResolveRenderPass() = default;

    void ResolveRenderPass::PrepareScene()
    {
    }

    void ResolveRenderPass::UpdateScene()
    {
        const auto& sceneProperties = transientContext->scene->GetSceneProperties();

        RefPtrVector<SceneV1::Node> lightNodes(sceneProperties.lightNodes.begin(), sceneProperties.lightNodes.end());

        UpdateLights(lightNodes);
    }

    void ResolveRenderPass::ExecuteGraphicsPass(
        RenderGraphResourceManager* manager,
        GraphicsCommandList* cmdList)
    {
        UpdateGBufferDescriptorSet(manager, transientContext->gbufferTextureDescriptorSet);
        UpdateIndirectLightingDescriptorSet(manager, transientContext->indirectLightingTexturesDescriptorSet);

        Viewport viewport;
        viewport.width = transientContext->outputSize.width;
        viewport.height = transientContext->outputSize.height;

        Rect2i scissorRect;
        scissorRect.size = transientContext->outputSize;

        cmdList->BindGraphicsPipelineState(persistentContext->pipelineState);
        cmdList->SetViewports({ viewport });
        cmdList->SetScissorRects({ scissorRect });

        cmdList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->indirectLightingTexturesDescriptorSet);

        cmdList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->gbufferTextureDescriptorSet);

        cmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->cameraBufferDescriptorSet);

        cmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            persistentContext->pipelineLayout,
            transientContext->lightsBufferDescriptorSet);

        cmdList->BindVertexBuffers({ persistentContext->fullscreenQuadBuffer }, { 0 });
        cmdList->Draw(0, fullscreenQuadVertices.size());
    }

    void ResolveRenderPass::UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes)
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

    void ResolveRenderPass::UpdateGBufferDescriptorSet(
        RenderGraphResourceManager* manager,
        DescriptorSet* descriptorSet)
    {
        for(int32 i = 0; i < gbuffer.color.size(); i++)
        {
            auto colorTexture = manager->GetTexture(gbuffer.color[i]);

            descriptorSet->WriteTexture(
                persistentContext->colorTextureBindings[i],
                colorTexture);
        }

        auto depthStencilTexture = manager->GetTexture(gbuffer.depthStencil);

        descriptorSet->WriteTexture(
            persistentContext->depthStencilTextureBinding,
            depthStencilTexture);

        descriptorSet->Update();
    }

    void ResolveRenderPass::UpdateIndirectLightingDescriptorSet(
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

    RefPtr<GraphicsPipelineState> ResolveRenderPass::CreateResolvePipelineState(ResolvePersistentContext* context)
    {
        GraphicsPipelineStateCreateInfo ci;

        ci.name = RenderPassName;

        auto& bindingDescription = ci.inputAssembler.bindings.emplace_back();
        bindingDescription.stride = sizeof(QuadVertex);
        bindingDescription.inputRate = VertexInputRate::PerVertex;

        auto& positionAttributeDescription = ci.inputAssembler.attributes.emplace_back();
        positionAttributeDescription.binding = 0;
        positionAttributeDescription.format = Format::RGB32Sfloat;
        positionAttributeDescription.offset = offsetof(QuadVertex, position);

        auto& texCoordAttributeDescription = ci.inputAssembler.attributes.emplace_back();
        texCoordAttributeDescription.binding = 0;
        texCoordAttributeDescription.format = Format::RG32Sfloat;
        texCoordAttributeDescription.offset = offsetof(QuadVertex, texCoord);

        ci.inputAssembler.primitiveTopology = PrimitiveTopology::TriangleList;
        ci.rasterization.cullMode = CullMode::Back;
        ci.rasterization.frontFace = FrontFace::CounterClockwise;

        ci.depthStencil.depthTestEnable = false;
        ci.depthStencil.depthWriteEnable = false;

        auto& colorAttachment = ci.colorAttachments.attachments.emplace_back();
        colorAttachment.format = ColorFormat;
        colorAttachment.blendEnable = false;

        ci.depthStencil.depthStencilFormat = Format::Undefined;

        //pipelineState.renderPass = lighting.renderPass;
        ci.pipelineLayout = context->pipelineLayout;
        ci.vertexProgram = context->vertexShader;
        ci.fragmentProgram = context->fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreateGraphicsPipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<ResolvePersistentContext>> ResolveRenderPass::PrepareResolvePersistentContext(
        GraphicsDevice* device,
        CameraResources* cameraResources,
        IndirectLightingResources* indirectLightingResources)
    {
        UniquePtr<ResolvePersistentContext> context = MakeUnique<ResolvePersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;
        context->indirectLightingResources = indirectLightingResources;

        {
            ColorAttachment colorAttachment;
            colorAttachment.format = ColorFormat;
            colorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
            colorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;

            RenderPassCreateInfo createInfo;
            createInfo.name = RenderPassName;
            createInfo.colorAttachments[0] = colorAttachment;

            auto[result, renderPass] = device->CreateRenderPass(createInfo);
            if(result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->renderPass = std::move(renderPass);
        }

        {
            auto [libraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
                context->device,
                "Data/Shaders/",
                "Data/Shaders/Deferred/",
                "resolve_vp",
                {});

            if (!libraryCreated)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            auto [result, vertexShaderProgram] = createdVertexShaderLibrary->CreateShaderProgram(ShaderStage::Vertex, "vp_main");
            if(result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->vertexShader = std::move(vertexShaderProgram);
        }

        {
            auto [libraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
                device,
                "Data/Shaders/",
                "Data/Shaders/Deferred/",
                "resolve_fp",
                {});

            if (!libraryCreated)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            auto [result, fragmentShaderProgram] = createdFragmentShaderLibrary->CreateShaderProgram(
                ShaderStage::Fragment,
                "fp_main");

            if(result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->fragmentShader = std::move(fragmentShaderProgram);
        }

        {
            context->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
            context->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .OfType(DescriptorSetType::Buffer)
                .WithNBindings(2)
                .AddBinding(&context->lightingParamsBinding)
                .AddBinding(&context->lightsBufferBinding);

            auto[result, descriptorSetLayout] = device->CreateDescriptorSetLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->lightsBufferDescriptorSetLayout = std::move(descriptorSetLayout);
        }

        {
            for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
            {
                context->colorTextureBindings[i].OfType(ResourceType::Texture);
            }
            
            context->depthStencilTextureBinding.OfType(ResourceType::Texture);

            DescriptorSetLayoutCreateInfo createInfo;
            createInfo
                .OfType(DescriptorSetType::Texture)
                .WithNBindings(DeferredConstants::GBufferColorAttachmentCount + 1);

            for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
            {
                createInfo.AddBinding(&context->colorTextureBindings[i]);
            }

            createInfo.AddBinding(&context->depthStencilTextureBinding);

            auto[result, descriptorSetLayout] = context->device->CreateDescriptorSetLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->gbufferTextureDescriptorSetLayout = std::move(descriptorSetLayout);
        }

        {
            PipelineLayoutCreateInfo createInfo;
            createInfo
                .AddSetLayout(ShaderStage::Fragment, context->cameraResources->cameraBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->lightsBufferDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->gbufferTextureDescriptorSetLayout)
                .AddSetLayout(ShaderStage::Fragment, context->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            auto[result, pipelineLayout] = context->device->CreatePipelineLayout(createInfo);
            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->pipelineLayout = std::move(pipelineLayout);
        }

        // TODO result
        context->pipelineState = CreateResolvePipelineState(context.get());

        {
            BufferCreateInfo createInfo;
            createInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
            createInfo.storageMode = ResourceStorageMode::Shared;
            createInfo.usage = BufferUsageFlags::VertexBuffer;

            auto[result, quadBuffer] = device->CreateBuffer(
                createInfo,
                fullscreenQuadVertices.data());

            if (result != GraphicsResult::Success)
            {
                LUCH_ASSERT(false);
                return { false };
            }

            context->fullscreenQuadBuffer = std::move(quadBuffer);
        }

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<ResolveTransientContext>> ResolveRenderPass::PrepareResolveTransientContext(
        ResolvePersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        UniquePtr<ResolveTransientContext> context = MakeUnique<ResolveTransientContext>();
        context->descriptorPool = descriptorPool;

        {
            auto [result, textureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->gbufferTextureDescriptorSetLayout);

            if (result != GraphicsResult::Success)
            {
                return { false };
            }

            context->gbufferTextureDescriptorSet = std::move(textureSet);
        }

        {
            auto [result, bufferSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->lightsBufferDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->lightsBufferDescriptorSet = std::move(bufferSet);
        }

        {
            auto [result, textureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->indirectLightingResources->indirectLightingTexturesDescriptorSetLayout);

            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->indirectLightingTexturesDescriptorSet = std::move(textureSet);
        }

        return { true, std::move(context) };
    }
}
