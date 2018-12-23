#include <Luch/Render/Deferred/ResolveRenderPass.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
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
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
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
        ResolveRenderContext* aContext,
        RenderGraphBuilder* builder)
        : context(aContext)
    {
        auto node = builder->AddRenderPass(RenderPassName, context->renderPass, this);

        for(int32 i = 0; i < context->gbuffer.color.size(); i++)
        {
            gbuffer.color[i] = node->ReadsTexture(context->gbuffer.color[i]);
        }

        gbuffer.depthStencil = node->ReadsTexture(context->gbuffer.depthStencil);

        resolveTextureHandle = node->CreateColorAttachment(0, context->attachmentSize);
    }

    ResolveRenderPass::~ResolveRenderPass() = default;

    void ResolveRenderPass::UpdateScene()
    {
        context->sharedBuffer->Reset();

        const auto& sceneProperties = context->scene->GetSceneProperties();

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

            context->gbufferTextureDescriptorSet->WriteTexture(
                context->colorTextureBindings[i],
                colorTexture);
        }

        auto depthStencilTexture = manager->GetTexture(gbuffer.gbufferDepthStencil);

        context->gbufferTextureDescriptorSet->WriteTexture(
            context->depthStencilTextureBinding,
            depthStencilTexture);

        context->gbufferTextureDescriptorSet->Update();

        Viewport viewport;
        viewport.width = context->attachmentSize.width;
        viewport.height = context->attachmentSize.height;

        Rect2i scissorRect;
        scissorRect.size = context->attachmentSize;

        cmdList->Begin();
        cmdList->BeginRenderPass(frameBuffer);
        cmdList->BindPipelineState(context->pipelineState);
        cmdList->SetViewports({ viewport });
        cmdList->SetScissorRects({ scissorRect });

        cmdList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            context->pipelineLayout,
            context->gbufferTextureDescriptorSet);

        cmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            context->pipelineLayout,
            camera->GetDescriptorSet("Deferred"));

        cmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            context->pipelineLayout,
            context->lightsBufferDescriptorSet);

        cmdList->BindVertexBuffers({ context->fullscreenQuadBuffer }, {0}, 0);
        cmdList->Draw(0, fullscreenQuadVertices.size());
        cmdList->EndRenderPass();
        cmdList->End();
    }

    void ResolveRenderPass::UpdateCamera(SceneV1::Camera* camera)
    {
        auto cameraUniform = RenderUtils::GetCameraUniform(camera, {});
        auto cameraUniformSuballocation = context->sharedBuffer->Suballocate(sizeof(CameraUniform), 16);

        memcpy(cameraUniformSuballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));

        context->lightsBufferDescriptorSet->WriteUniformBuffer(
            context->cameraUniformBufferBinding,
            cameraUniformSuballocation.buffer,
            cameraUniformSuballocation.offset);

        context->cameraBufferDescriptorSet->Update();
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

        auto lightingParamsSuballocation = context->sharedBuffer->Suballocate(sizeof(LightingParamsUniform), 16);
        auto lightsSuballocation = context->sharedBuffer->Suballocate(enabledLightsCount * sizeof(LightUniform), 16);

        memcpy(lightingParamsSuballocation.offsetMemory, &lightingParams, sizeof(LightingParamsUniform));
        memcpy(lightsSuballocation.offsetMemory, lightUniforms.data(), enabledLightsCount * sizeof(LightUniform));

        context->lightsBufferDescriptorSet->WriteUniformBuffer(
            context->lightingParamsBinding,
            lightingParamsSuballocation.buffer,
            lightingParamsSuballocation.offset);

        context->lightsBufferDescriptorSet->WriteUniformBuffer(
            context->lightsBufferBinding,
            lightsSuballocation.buffer,
            lightsSuballocation.offset);

        context->lightsBufferDescriptorSet->Update();
    }

    RefPtr<PipelineState> ResolveRenderPass::CreateResolvePipelineState(ResolveRenderContext* context)
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

    ResultValue<bool, UniquePtr<ResolveRenderContext>> ResolveRenderPass::PrepareResolveRenderContext(GraphicsDevice* device)
    {
        UniquePtr<ResolveRenderContext> context = MakeUnique<ResolveRenderContext>();
        context->device = device;

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

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 4;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, 2 },
            { ResourceType::Texture, OffscreenImageCount + 1 },
            { ResourceType::Sampler, OffscreenImageCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = device->CreateDescriptorPool(
            descriptorPoolCreateInfo);
        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->descriptorPool = std::move(createdDescriptorPool);

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\resolve.vert",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/resolve_vp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/resolve.vert",
    #else
            "",
    #endif
#endif
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
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\resolve.frag",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/resolve_fp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/resolve.frag",
    #else
            "",
    #endif
#endif
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

        context->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&context->cameraUniformBufferBinding);

        context->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        context->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = device->CreateDescriptorSetLayout(cameraDescriptorSetLayoutCreateInfo);
        if (createCameraDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->cameraBufferDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

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
            .AddSetLayout(ShaderStage::Fragment, context->cameraBufferDescriptorSetLayout)
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
        context->pipelineState = CreateResolvePipelineState(context);

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

        auto [allocateTextureSetResult, allocatedTextureSet] = context->descriptorPool->AllocateDescriptorSet(
                context->gbufferTextureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->gbufferTextureDescriptorSet = std::move(allocatedTextureSet);

        auto [allocateCameraDescriptorSetResult, allocatedCameraBufferSet] = context->descriptorPool->AllocateDescriptorSet(
            context->cameraBufferDescriptorSetLayout);

        if(allocateCameraDescriptorSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->cameraBufferDescriptorSet = allocatedCameraBufferSet;

        auto [allocateLightsDescriptorSetResult, allocatedLightsBufferSet] = context->descriptorPool->AllocateDescriptorSet(
            context->lightsBufferDescriptorSetLayout);

        if(allocateLightsDescriptorSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->lightsBufferDescriptorSet = allocatedLightsBufferSet;

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedUniformBufferSize;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = device->CreateBuffer(bufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->sharedBuffer = MakeUnique<SharedBuffer>(std::move(createdBuffer));

        return { true, std::move(context) };
    }
}
