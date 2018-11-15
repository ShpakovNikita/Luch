#include <Luch/Render/Deferred/ResolveRenderer.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Deferred/DeferredResources.h>
#include <Luch/Render/Deferred/GBufferTextures.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/Light.h>

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
#include <Luch/Graphics/SamplerCreateInfo.h>
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

namespace Luch::Render::Deferred
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
    constexpr Array<QuadVertex, 3> fullscreenQuadVertices =
    {
        QuadVertex { Vec3{-1.0f, -1.0f, 0.0f}, Vec2{0.0f, +1.0f} },
        QuadVertex { Vec3{+3.0f, -1.0f, 0.0f}, Vec2{2.0f, +1.0f} },
        QuadVertex { Vec3{-1.0f, +3.0f, 0.0f}, Vec2{0.0f, -1.0f} },
    };

    const String ResolveRenderer::RendererName{"Resolve"};

    ResolveRenderer::ResolveRenderer() = default;
    ResolveRenderer::~ResolveRenderer() = default;

    bool ResolveRenderer::Initialize()
    {
        HUSKY_ASSERT(context != nullptr);
        HUSKY_ASSERT(commonResources != nullptr);

        auto[resourcesPrepared, preparedResources] = PrepareResolvePassResources();
        if (!resourcesPrepared)
        {
            return false;
        }

        resources = std::move(preparedResources);

        auto [resolveTextureCreated, createdResolveTexture] = CreateResolveTexture();
        if(!resolveTextureCreated)
        {
            return false;
        }

        resolveTexture = std::move(createdResolveTexture);

        return true;
    }

    bool ResolveRenderer::Deinitialize()
    {
        context.reset();
        commonResources.reset();
        resources.reset();

        return true;
    }

    void ResolveRenderer::PrepareScene(SceneV1::Scene* scene)
    {
    }

    void ResolveRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        resources->sharedBuffer->Reset();

        const auto& sceneProperties = scene->GetSceneProperties();

        RefPtrVector<SceneV1::Node> lightNodes(sceneProperties.lightNodes.begin(), sceneProperties.lightNodes.end());

        UpdateLights(lightNodes);
    }

    Texture* ResolveRenderer::Resolve(
        SceneV1::Scene* scene,
        SceneV1::Camera* camera,
        GBufferTextures* gbuffer)
    {
        resources->gbufferTextureDescriptorSet->WriteTexture(
            resources->baseColorTextureBinding,
            gbuffer->baseColorTexture);

        resources->gbufferTextureDescriptorSet->WriteTexture(
            resources->normalMapTextureBinding,
            gbuffer->normalMapTexture);

        resources->gbufferTextureDescriptorSet->WriteTexture(
            resources->depthStencilTextureBinding,
            gbuffer->depthStencilBuffer);

        resources->gbufferTextureDescriptorSet->Update();

        auto [allocateCmdListResult, cmdList] = resources->commandPool->AllocateGraphicsCommandList();
        HUSKY_ASSERT(allocateCmdListResult == GraphicsResult::Success);

        ColorAttachment colorAttachment = resources->colorAttachmentTemplate;
        colorAttachment.output.texture = resolveTexture;

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport {
            0, 0, static_cast<float32>(framebufferWidth), static_cast<float32>(framebufferHeight), 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
            .WithName(RendererName)
            .WithNColorAttachments(1)
            .AddColorAttachment(&colorAttachment);

        cmdList->Begin();
        cmdList->BeginRenderPass(renderPassCreateInfo);
        cmdList->BindPipelineState(resources->pipelineState);
        cmdList->SetViewports({ viewport });
        cmdList->SetScissorRects({ scissorRect });

        cmdList->BindTextureDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            resources->gbufferTextureDescriptorSet);

        cmdList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            resources->gbufferSamplerDescriptorSet);

        cmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            camera->GetDescriptorSet("Deferred"));

        cmdList->BindBufferDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            resources->lightsBufferDescriptorSet);

        cmdList->BindVertexBuffers({ resources->fullscreenQuadBuffer }, {0}, 0);
        cmdList->Draw(0, fullscreenQuadVertices.size());
        cmdList->EndRenderPass();
        cmdList->End();

        context->commandQueue->Submit(cmdList);

        return resolveTexture;
    }

    void ResolveRenderer::UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes)
    {
        Vector<LightUniform> lightUniforms;

        for(const auto& lightNode : lightNodes)
        {
            const auto& light = lightNode->GetLight();
            HUSKY_ASSERT(light != nullptr);

            if(light->IsEnabled())
            {
                LightUniform lightUniform = RenderUtils::GetLightUniform(light, lightNode->GetWorldTransform());
                lightUniforms.push_back(lightUniform);
            }
        }

        int32 enabledLightsCount = lightNodes.size();

        LightingParamsUniform lightingParams;
        lightingParams.lightCount = enabledLightsCount;

        auto lightingParamsSuballocation = resources->sharedBuffer->Suballocate(sizeof(LightingParamsUniform), 16);
        auto lightsSuballocation = resources->sharedBuffer->Suballocate(enabledLightsCount * sizeof(LightUniform), 16);

        memcpy(lightingParamsSuballocation.offsetMemory, &lightingParams, sizeof(LightingParamsUniform));
        memcpy(lightsSuballocation.offsetMemory, lightUniforms.data(), enabledLightsCount * sizeof(LightUniform));

        resources->lightsBufferDescriptorSet->WriteUniformBuffer(
            resources->lightingParamsBinding,
            lightingParamsSuballocation.buffer,
            lightingParamsSuballocation.offset);

        resources->lightsBufferDescriptorSet->WriteUniformBuffer(
            resources->lightsBufferBinding,
            lightsSuballocation.buffer,
            lightsSuballocation.offset);

        resources->lightsBufferDescriptorSet->Update();
    }

    RefPtr<PipelineState> ResolveRenderer::CreateResolvePipelineState(ResolvePassResources* resolve)
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
        colorAttachment.format = colorFormat;
        colorAttachment.blendEnable = false;

        ci.depthStencil.depthStencilFormat = Format::Undefined;

        //pipelineState.renderPass = lighting.renderPass;
        ci.pipelineLayout = resolve->pipelineLayout;
        ci.vertexProgram = resolve->vertexShader;
        ci.fragmentProgram = resolve->fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<ResolvePassResources>> ResolveRenderer::PrepareResolvePassResources()
    {
        UniquePtr<ResolvePassResources> resolveResources = MakeUnique<ResolvePassResources>();

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 3;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, 1 },
            { ResourceType::Texture, OffscreenImageCount + 1 },
            { ResourceType::Sampler, OffscreenImageCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
            descriptorPoolCreateInfo);
        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->descriptorPool = std::move(createdDescriptorPool);

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        resolveResources->commandPool = std::move(createdCommandPool);

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\resolve.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/resolve_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/resolve.vert",
    #else
            "",
    #endif
#endif
            {});

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

        resolveResources->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\resolve.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/resolve_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/resolve.frag",
    #else
            "",
    #endif
#endif
            {});

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

        resolveResources->fragmentShader = std::move(fragmentShaderProgram);

        resolveResources->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        resolveResources->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(2)
            .AddBinding(&resolveResources->lightingParamsBinding)
            .AddBinding(&resolveResources->lightsBufferBinding);

        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);

        resolveResources->baseColorTextureBinding.OfType(ResourceType::Texture);
        resolveResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);
        resolveResources->normalMapTextureBinding.OfType(ResourceType::Texture);
        resolveResources->normalMapSamplerBinding.OfType(ResourceType::Sampler);
        resolveResources->depthStencilTextureBinding.OfType(ResourceType::Texture);
        resolveResources->depthStencilSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
        gbufferTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(3)
            .AddBinding(&resolveResources->baseColorTextureBinding)
            .AddBinding(&resolveResources->normalMapTextureBinding)
            .AddBinding(&resolveResources->depthStencilTextureBinding);

        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo gbufferSamplerDescriptorSetLayoutCreateInfo;
        gbufferSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(3)
            .AddBinding(&resolveResources->baseColorSamplerBinding)
            .AddBinding(&resolveResources->normalMapSamplerBinding)
            .AddBinding(&resolveResources->depthStencilSamplerBinding);

        auto[createGBufferSamplerDescriptorSetLayoutResult, createdGBufferSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferSamplerDescriptorSetLayoutCreateInfo);
        if (createGBufferSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->gbufferSamplerDescriptorSetLayout = std::move(createdGBufferSamplerDescriptorSetLayout);

        SamplerCreateInfo samplerCreateInfo;

        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createBaseColorSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->baseColorSampler = std::move(createdBaseColorSampler);

        auto[createNormalMapSamplerResult, createdNormalMapSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createNormalMapSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->normalMapSampler = std::move(createdNormalMapSampler);

        auto[createDepthSamplerResult, createdDepthSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createDepthSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->depthStencilSampler = std::move(createdDepthSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, commonResources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resolveResources->lightsBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resolveResources->gbufferTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resolveResources->gbufferSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->pipelineLayout = std::move(createdPipelineLayout);

        resolveResources->colorAttachmentTemplate.format = colorFormat;
        resolveResources->colorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        resolveResources->colorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;

        // TODO result
        resolveResources->pipelineState = CreateResolvePipelineState(resolveResources.get());

        BufferCreateInfo quadBufferCreateInfo;
        quadBufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
        quadBufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        quadBufferCreateInfo.usage = BufferUsageFlags::VertexBuffer;

        auto[createQuadBufferResult, createdQuadBuffer] = context->device->CreateBuffer(
            quadBufferCreateInfo,
            fullscreenQuadVertices.data());

        if (createQuadBufferResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->fullscreenQuadBuffer = std::move(createdQuadBuffer);

        auto [allocateTextureSetResult, allocatedTextureSet] = resolveResources->descriptorPool->AllocateDescriptorSet(
                resolveResources->gbufferTextureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        resolveResources->gbufferTextureDescriptorSet = std::move(allocatedTextureSet);

        auto [allocateSamplerSetResult, allocatedSamplerSet] = resolveResources->descriptorPool->AllocateDescriptorSet(
            resolveResources->gbufferSamplerDescriptorSetLayout);

        if (allocateSamplerSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        resolveResources->gbufferSamplerDescriptorSet = std::move(allocatedSamplerSet);

        resolveResources->gbufferSamplerDescriptorSet->WriteSampler(
            resolveResources->baseColorSamplerBinding,
            resolveResources->baseColorSampler);

        resolveResources->gbufferSamplerDescriptorSet->WriteSampler(
            resolveResources->normalMapSamplerBinding,
            resolveResources->normalMapSampler);

        resolveResources->gbufferSamplerDescriptorSet->WriteSampler(
            resolveResources->depthStencilSamplerBinding,
            resolveResources->depthStencilSampler);

        resolveResources->gbufferSamplerDescriptorSet->Update();

        auto [createDescriptorSetResult, createdDescriptorSet] = resolveResources->descriptorPool->AllocateDescriptorSet(
            resolveResources->lightsBufferDescriptorSetLayout);

        if(createDescriptorSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        resolveResources->lightsBufferDescriptorSet = createdDescriptorSet;

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedUniformBufferSize;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->sharedBuffer = MakeUnique<SharedBuffer>(std::move(createdBuffer));

        return { true, std::move(resolveResources) };
    }

    ResultValue<bool, RefPtr<Texture>> ResolveRenderer::CreateResolveTexture()
    {
        const auto& swapchainCreateInfo = context->swapchain->GetInfo();
        int32 textureWidth = swapchainCreateInfo.width;
        int32 textureHeight = swapchainCreateInfo.height;

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = colorFormat;
        textureCreateInfo.width = textureWidth;
        textureCreateInfo.height = textureHeight;
        textureCreateInfo.usage =
              TextureUsageFlags::ColorAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        auto[createTextureResult, createdTexture] = context->device->CreateTexture(
            textureCreateInfo);

        if (createTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        return { true, std::move(createdTexture) };
    }
}
