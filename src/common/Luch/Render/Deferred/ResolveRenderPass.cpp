#include <Luch/Render/Deferred/ResolveRenderPass.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/Deferred/ResolveContext.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/Light.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/SamplerCreateInfo.h>
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/FrameBuffer.h>
#include <Luch/Graphics/Texture.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/PipelineState.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/Graphics/PipelineStateCreateInfo.h>

namespace Luch::Render::Deferred
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
        auto node = builder->AddRenderPass(RenderPassName, persistentContext->renderPass, this);

        for(int32 i = 0; i < transientContext->gbuffer.color.size(); i++)
        {
            gbuffer.color[i] = node->ReadsTexture(transientContext->gbuffer.color[i]);
        }

        gbuffer.depthStencil = node->ReadsTexture(transientContext->gbuffer.depthStencil);

        resolveTextureHandle = node->CreateColorAttachment(0, transientContext->attachmentSize);
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

    void ResolveRenderPass::ExecuteRenderPass(
        RenderGraphResourceManager* manager,
        FrameBuffer* frameBuffer, 
        GraphicsCommandList* cmdList)
    {
        for(int32 i = 0; i < gbuffer.color.size(); i++)
        {
            auto colorTexture = manager->GetTexture(gbuffer.color[i]);

            transientContext->gbufferTextureDescriptorSet->WriteTexture(
                persistentContext->colorTextureBindings[i],
                colorTexture);
        }

        auto depthStencilTexture = manager->GetTexture(gbuffer.depthStencil);

        transientContext->gbufferTextureDescriptorSet->WriteTexture(
            persistentContext->depthStencilTextureBinding,
            depthStencilTexture);

        transientContext->gbufferTextureDescriptorSet->Update();

        Viewport viewport;
        viewport.width = transientContext->attachmentSize.width;
        viewport.height = transientContext->attachmentSize.height;

        Rect2i scissorRect;
        scissorRect.size = transientContext->attachmentSize;

        cmdList->Begin();
        cmdList->BeginRenderPass(frameBuffer);
        cmdList->BindPipelineState(persistentContext->pipelineState);
        cmdList->SetViewports({ viewport });
        cmdList->SetScissorRects({ scissorRect });

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

        cmdList->BindVertexBuffers({ persistentContext->fullscreenQuadBuffer }, {0});
        cmdList->Draw(0, fullscreenQuadVertices.size());
        cmdList->EndRenderPass();
        cmdList->End();
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

    RefPtr<PipelineState> ResolveRenderPass::CreateResolvePipelineState(ResolvePersistentContext* context)
    {
        PipelineStateCreateInfo ci;

        ci.name = "Resolve";

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

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<ResolvePersistentContext>> ResolveRenderPass::PrepareResolvePersistentContext(
        GraphicsDevice* device,
        CameraResources* cameraResources)
    {
        UniquePtr<ResolvePersistentContext> context = MakeUnique<ResolvePersistentContext>();
        context->device = device;
        context->cameraResources = cameraResources;

        ColorAttachment colorAttachment;
        colorAttachment.format = ColorFormat;
        colorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
        colorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.name = RenderPassName;
        renderPassCreateInfo.colorAttachments[0] = colorAttachment;

        auto[createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
        if(createRenderPassResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->renderPass = std::move(createdRenderPass);

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
            "Data/Shaders/Deferred/",
            "resolve_vp",
            {});

        if (!vertexShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        auto [vertexShaderProgramCreateResult, vertexShaderProgram] = createdVertexShaderLibrary->CreateShaderProgram(ShaderStage::Vertex, "vp_main");
        if(vertexShaderProgramCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            device,
            "Data/Shaders/Deferred/",
            "resolve_fp",
            {});

        if (!fragmentShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        auto [fragmentShaderProgramCreateResult, fragmentShaderProgram] = createdFragmentShaderLibrary->CreateShaderProgram(
            ShaderStage::Fragment,
            "fp_main");

        if(fragmentShaderProgramCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->fragmentShader = std::move(fragmentShaderProgram);

        context->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        context->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

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

        for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
        {
            context->colorTextureBindings[i].OfType(ResourceType::Texture);
        }
        
        context->depthStencilTextureBinding.OfType(ResourceType::Texture);

        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
        gbufferTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(DeferredConstants::GBufferColorAttachmentCount + 1);
        for(int32 i = 0; i < DeferredConstants::GBufferColorAttachmentCount; i++)
        {
            gbufferTextureDescriptorSetLayoutCreateInfo.AddBinding(&context->colorTextureBindings[i]);
        }

        gbufferTextureDescriptorSetLayoutCreateInfo.AddBinding(&context->depthStencilTextureBinding);

        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, context->cameraResources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, context->lightsBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, context->gbufferTextureDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->pipelineLayout = std::move(createdPipelineLayout);

        // TODO result
        context->pipelineState = CreateResolvePipelineState(context.get());

        BufferCreateInfo quadBufferCreateInfo;
        quadBufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
        quadBufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        quadBufferCreateInfo.usage = BufferUsageFlags::VertexBuffer;

        auto[createQuadBufferResult, createdQuadBuffer] = device->CreateBuffer(
            quadBufferCreateInfo,
            fullscreenQuadVertices.data());

        if (createQuadBufferResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->fullscreenQuadBuffer = std::move(createdQuadBuffer);

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<ResolveTransientContext>> ResolveRenderPass::PrepareResolveTransientContext(
        ResolvePersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        UniquePtr<ResolveTransientContext> context = MakeUnique<ResolveTransientContext>();
        context->descriptorPool = descriptorPool;

        auto [allocateTextureSetResult, allocatedTextureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->gbufferTextureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->gbufferTextureDescriptorSet = std::move(allocatedTextureSet);

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
