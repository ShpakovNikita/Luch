#include <Husky/Render/Deferred/LightingRenderer.h>
#include <Husky/Render/RenderContext.h>
#include <Husky/Render/RenderUtils.h>
#include <Husky/Render/SharedBuffer.h>
#include <Husky/Render/Deferred/DeferredResources.h>
#include <Husky/Render/Deferred/GBufferTextures.h>
#include <Husky/Render/Deferred/LightingTextures.h>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/Light.h>

#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Graphics/TextureCreateInfo.h>
#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/ShaderLibrary.h>
#include <Husky/Graphics/DescriptorSet.h>
#include <Husky/Graphics/PhysicalDevice.h>
#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/Graphics/CommandQueue.h>
#include <Husky/Graphics/CommandPool.h>
#include <Husky/Graphics/DescriptorPool.h>
#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Graphics/SamplerCreateInfo.h>
#include <Husky/Graphics/Swapchain.h>
#include <Husky/Graphics/SwapchainInfo.h>
#include <Husky/Graphics/PipelineState.h>
#include <Husky/Graphics/PrimitiveTopology.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/RenderPassCreateInfo.h>
#include <Husky/Graphics/DescriptorPoolCreateInfo.h>
#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Graphics/PipelineLayoutCreateInfo.h>
#include <Husky/Graphics/IndexType.h>
#include <Husky/Graphics/PipelineStateCreateInfo.h>

namespace Husky::Render::Deferred
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

    const String LightingRenderer::RendererName{"Lighting"};

    LightingRenderer::LightingRenderer() = default;
    LightingRenderer::~LightingRenderer() = default;

    bool LightingRenderer::Initialize()
    {
        HUSKY_ASSERT(context != nullptr);
        HUSKY_ASSERT(commonResources != nullptr);

        auto[lightingResourcesPrepared, preparedLightingResources] = PrepareLightingPassResources();
        if (!lightingResourcesPrepared)
        {
            return false;
        }

        resources = std::move(preparedLightingResources);

        auto [lightingTexturesCreated, createdLightingTextures] = CreateLightingTextures();
        if(!lightingTexturesCreated)
        {
            return false;
        }

        lightingTextures = std::move(createdLightingTextures);

        return true;
    }

    bool LightingRenderer::Deinitialize()
    {
        context.reset();
        commonResources.reset();
        resources.reset();

        return true;
    }

    void LightingRenderer::PrepareScene(SceneV1::Scene* scene)
    {
    }

    void LightingRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        resources->sharedBuffer->Reset();

        const auto& sceneProperties = scene->GetSceneProperties();

        UpdateLights(sceneProperties.lightNodes);
    }

    LightingTextures* LightingRenderer::DrawScene(
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

        ColorAttachment diffuseAttachment = resources->diffuseAttachmentTemplate;
        diffuseAttachment.output.texture = lightingTextures->diffuseLightingTexture;

        ColorAttachment specularAttachment = resources->specularAttachmentTemplate;
        specularAttachment.output.texture = lightingTextures->specularLightingTexture;

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport {
            0, 0, static_cast<float32>(framebufferWidth), static_cast<float32>(framebufferHeight), 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        RenderPassCreateInfo lightingRenderPassCreateInfo;
        lightingRenderPassCreateInfo
            .WithNColorAttachments(2)
            .AddColorAttachment(&diffuseAttachment)
            .AddColorAttachment(&specularAttachment);

        cmdList->Begin();
        cmdList->BeginRenderPass(lightingRenderPassCreateInfo);
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

        return lightingTextures.get();
    }

    void LightingRenderer::UpdateLights(const RefPtrUnorderedSet<SceneV1::Node>& lightNodes)
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

    RefPtr<PipelineState> LightingRenderer::CreateLightingPipelineState(LightingPassResources* lighting)
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

        auto& diffuseAttachment = ci.colorAttachments.attachments.emplace_back();
        diffuseAttachment.format = diffuseFormat;
        diffuseAttachment.blendEnable = false;

        auto& specularAttachment = ci.colorAttachments.attachments.emplace_back();
        specularAttachment.format = specularFormat;
        specularAttachment.blendEnable = false;

        ci.depthStencil.depthStencilFormat = Format::Undefined;

        //pipelineState.renderPass = lighting.renderPass;
        ci.pipelineLayout = lighting->pipelineLayout;
        ci.vertexProgram = lighting->vertexShader;
        ci.fragmentProgram = lighting->fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<LightingPassResources>> LightingRenderer::PrepareLightingPassResources()
    {
        UniquePtr<LightingPassResources> lightingResources = MakeUnique<LightingPassResources>();

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

        lightingResources->descriptorPool = std::move(createdDescriptorPool);

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        lightingResources->commandPool = std::move(createdCommandPool);

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.vert",
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

        lightingResources->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.frag",
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

        lightingResources->fragmentShader = std::move(fragmentShaderProgram);

        lightingResources->lightingParamsBinding.OfType(ResourceType::UniformBuffer);
        lightingResources->lightsBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(2)
            .AddBinding(&lightingResources->lightingParamsBinding)
            .AddBinding(&lightingResources->lightsBufferBinding);

        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);

        lightingResources->baseColorTextureBinding.OfType(ResourceType::Texture);
        lightingResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);
        lightingResources->normalMapTextureBinding.OfType(ResourceType::Texture);
        lightingResources->normalMapSamplerBinding.OfType(ResourceType::Sampler);
        lightingResources->depthStencilTextureBinding.OfType(ResourceType::Texture);
        lightingResources->depthStencilSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
        gbufferTextureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(3)
            .AddBinding(&lightingResources->baseColorTextureBinding)
            .AddBinding(&lightingResources->normalMapTextureBinding)
            .AddBinding(&lightingResources->depthStencilTextureBinding);

        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo gbufferSamplerDescriptorSetLayoutCreateInfo;
        gbufferSamplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(3)
            .AddBinding(&lightingResources->baseColorSamplerBinding)
            .AddBinding(&lightingResources->normalMapSamplerBinding)
            .AddBinding(&lightingResources->depthStencilSamplerBinding);

        auto[createGBufferSamplerDescriptorSetLayoutResult, createdGBufferSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferSamplerDescriptorSetLayoutCreateInfo);
        if (createGBufferSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->gbufferSamplerDescriptorSetLayout = std::move(createdGBufferSamplerDescriptorSetLayout);

        SamplerCreateInfo samplerCreateInfo;

        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createBaseColorSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->baseColorSampler = std::move(createdBaseColorSampler);

        auto[createNormalMapSamplerResult, createdNormalMapSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createNormalMapSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->normalMapSampler = std::move(createdNormalMapSampler);

        auto[createDepthSamplerResult, createdDepthSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createDepthSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->depthStencilSampler = std::move(createdDepthSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, commonResources->cameraBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, lightingResources->lightsBufferDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, lightingResources->gbufferTextureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, lightingResources->gbufferSamplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        lightingResources->pipelineLayout = std::move(createdPipelineLayout);

        lightingResources->diffuseAttachmentTemplate.format = diffuseFormat;
        lightingResources->diffuseAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        lightingResources->diffuseAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;

        lightingResources->specularAttachmentTemplate.format = specularFormat;
        lightingResources->specularAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        lightingResources->specularAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;

        // TODO result
        lightingResources->pipelineState = CreateLightingPipelineState(lightingResources.get());

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

        lightingResources->fullscreenQuadBuffer = std::move(createdQuadBuffer);

        auto [allocateTextureSetResult, allocatedTextureSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
                lightingResources->gbufferTextureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        lightingResources->gbufferTextureDescriptorSet = std::move(allocatedTextureSet);

        auto [allocateSamplerSetResult, allocatedSamplerSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
            lightingResources->gbufferSamplerDescriptorSetLayout);

        if (allocateSamplerSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        lightingResources->gbufferSamplerDescriptorSet = std::move(allocatedSamplerSet);

        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
            lightingResources->baseColorSamplerBinding,
            lightingResources->baseColorSampler);

        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
            lightingResources->normalMapSamplerBinding,
            lightingResources->normalMapSampler);

        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
            lightingResources->depthStencilSamplerBinding,
            lightingResources->depthStencilSampler);

        lightingResources->gbufferSamplerDescriptorSet->Update();

        auto [createDescriptorSetResult, createdDescriptorSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
            lightingResources->lightsBufferDescriptorSetLayout);

        if(createDescriptorSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        lightingResources->lightsBufferDescriptorSet = createdDescriptorSet;

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

        lightingResources->sharedBuffer = MakeUnique<SharedBuffer>(std::move(createdBuffer));

        return { true, std::move(lightingResources) };
    }

    ResultValue<bool, UniquePtr<LightingTextures>> LightingRenderer::CreateLightingTextures()
    {
        UniquePtr<LightingTextures> textures = MakeUnique<LightingTextures>();

        const auto& swapchainCreateInfo = context->swapchain->GetInfo();
        int32 textureWidth = swapchainCreateInfo.width;
        int32 textureHeight = swapchainCreateInfo.height;

        TextureCreateInfo diffuseTextureCreateInfo;
        diffuseTextureCreateInfo.format = diffuseFormat;
        diffuseTextureCreateInfo.width = textureWidth;
        diffuseTextureCreateInfo.height = textureHeight;
        diffuseTextureCreateInfo.usage =
              TextureUsageFlags::ColorAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        auto[createDiffuseTextureResult, createdDiffuseTexture] = context->device->CreateTexture(
            diffuseTextureCreateInfo);

        if (createDiffuseTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures->diffuseLightingTexture = std::move(createdDiffuseTexture);

        TextureCreateInfo specularTextureCreateInfo;
        specularTextureCreateInfo.format = specularFormat;
        specularTextureCreateInfo.width = textureWidth;
        specularTextureCreateInfo.height = textureHeight;
        specularTextureCreateInfo.usage =
              TextureUsageFlags::ColorAttachment
            | TextureUsageFlags::ShaderRead
            | TextureUsageFlags::TransferSource;

        auto[createSpecularTextureResult, createdSpecularTexture] = context->device->CreateTexture(
            specularTextureCreateInfo);

        if (createSpecularTextureResult != GraphicsResult::Success)
        {
            return { false };
        }

        textures->specularLightingTexture = std::move(createdSpecularTexture);

        return { true, std::move(textures) };
    }
}
