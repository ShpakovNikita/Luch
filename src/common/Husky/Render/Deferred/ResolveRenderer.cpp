#include <Husky/Render/Deferred/ResolveRenderer.h>
#include <Husky/Render/RenderContext.h>
#include <Husky/Render/RenderUtils.h>
#include <Husky/Render/SharedBuffer.h>
#include <Husky/Render/Deferred/GBufferTextures.h>
#include <Husky/Render/Deferred/LightingTextures.h>
#include <Husky/Render/Deferred/ResolvePassResources.h>

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

        auto[resolveResourcesPrepared, preparedResolveResources] = PrepareResolvePassResources(context.get());
        if (!resolveResourcesPrepared)
        {
            return false;
        }

        resources = std::move(preparedResolveResources);

        return true;
    }

    bool ResolveRenderer::Deinitialize()
    {
        context.reset();
        resources.reset();

        return true;
    }

    void ResolveRenderer::Resolve(Texture* output, GBufferTextures* gbuffer, LightingTextures* lightingTextures)
    {
        resources->textureDescriptorSet->WriteTexture(
            resources->baseColorTextureBinding,
            gbuffer->baseColorTexture);

        resources->textureDescriptorSet->WriteTexture(
            resources->diffuseTextureBinding,
            lightingTextures->diffuseLightingTexture);

        resources->textureDescriptorSet->WriteTexture(
            resources->specularTextureBinding,
            lightingTextures->specularLightingTexture);

        resources->textureDescriptorSet->Update();

        auto [allocateCmdListResult, cmdList] = resources->commandPool->AllocateGraphicsCommandList();
        HUSKY_ASSERT(allocateCmdListResult == GraphicsResult::Success);

        ColorAttachment colorAttachment = resources->colorAttachmentTemplate;
        colorAttachment.output.texture = output;

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport {
            0, 0, static_cast<float32>(framebufferWidth), static_cast<float32>(framebufferHeight), 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
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
            resources->textureDescriptorSet);

        cmdList->BindSamplerDescriptorSet(
            ShaderStage::Fragment,
            resources->pipelineLayout,
            resources->samplerDescriptorSet);

        cmdList->BindVertexBuffers({ resources->fullscreenQuadBuffer }, {0}, 0);
        cmdList->Draw(0, fullscreenQuadVertices.size());
        cmdList->EndRenderPass();
        cmdList->End();

        context->commandQueue->Submit(cmdList);
    }

    RefPtr<PipelineState> ResolveRenderer::CreateResolvePipelineState(
        RenderContext* context,
        ResolvePassResources* resources)
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
        colorAttachment.format = context->swapchain->GetInfo().format;
        colorAttachment.blendEnable = false;

        ci.depthStencil.depthStencilFormat = Format::Undefined;

        //pipelineState.renderPass = lighting.renderPass;
        ci.pipelineLayout = resources->pipelineLayout;
        ci.vertexProgram = resources->vertexShader;
        ci.fragmentProgram = resources->fragmentShader;

        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<ResolvePassResources>> ResolveRenderer::PrepareResolvePassResources(
        RenderContext* context)
    {
        UniquePtr<ResolvePassResources> resolveResources = MakeUnique<ResolvePassResources>();

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.maxDescriptorSets = 3;
        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, 1 },
            { ResourceType::Texture, 3 },
            { ResourceType::Sampler, 3 },
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
#if _WIN32 && HUSKY_USE_VULKAN
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\resolve.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/resolve_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/resolve.vert",
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

        auto [vertexShaderProgramCreateResult, vertexShaderProgram] = createdVertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex, "vp_main");
        if(vertexShaderProgramCreateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32 && HUSKY_USE_VULKAN
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\resolve.frag",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/resolve_fp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/resolve.frag",
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

        resolveResources->baseColorTextureBinding.OfType(ResourceType::Texture);
        resolveResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);
        resolveResources->diffuseTextureBinding.OfType(ResourceType::Texture);
        resolveResources->diffuseSamplerBinding.OfType(ResourceType::Sampler);
        resolveResources->specularTextureBinding.OfType(ResourceType::Texture);
        resolveResources->specularSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo textureDescriptorSetLayoutCreateInfo;
        textureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(3)
            .AddBinding(&resolveResources->baseColorTextureBinding)
            .AddBinding(&resolveResources->diffuseTextureBinding)
            .AddBinding(&resolveResources->specularTextureBinding);

        auto[createTextureDescriptorSetLayoutResult, createdTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            textureDescriptorSetLayoutCreateInfo);

        if (createTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->textureDescriptorSetLayout = std::move(createdTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo samplerDescriptorSetLayoutCreateInfo;
        samplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(3)
            .AddBinding(&resolveResources->baseColorSamplerBinding)
            .AddBinding(&resolveResources->diffuseSamplerBinding)
            .AddBinding(&resolveResources->specularSamplerBinding);

        auto[createSamplerDescriptorSetLayoutResult, createdSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            samplerDescriptorSetLayoutCreateInfo);

        if (createSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->samplerDescriptorSetLayout = std::move(createdSamplerDescriptorSetLayout);

        SamplerCreateInfo samplerCreateInfo;

        auto[createSamplerResult, createdSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createSamplerResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->sampler = std::move(createdSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, resolveResources->textureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, resolveResources->samplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        resolveResources->pipelineLayout = std::move(createdPipelineLayout);

        resolveResources->colorAttachmentTemplate.format = context->swapchain->GetInfo().format;
        resolveResources->colorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
        resolveResources->colorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;

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
            resolveResources->textureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        resolveResources->textureDescriptorSet = std::move(allocatedTextureSet);

        auto [allocateSamplerSetResult, allocatedSamplerSet] = resolveResources->descriptorPool->AllocateDescriptorSet(
            resolveResources->samplerDescriptorSetLayout);

        if (allocateSamplerSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        resolveResources->samplerDescriptorSet = std::move(allocatedSamplerSet);

        resolveResources->samplerDescriptorSet->WriteSampler(
            resolveResources->baseColorSamplerBinding,
            resolveResources->sampler);

        resolveResources->samplerDescriptorSet->WriteSampler(
            resolveResources->diffuseSamplerBinding,
            resolveResources->sampler);

        resolveResources->samplerDescriptorSet->WriteSampler(
            resolveResources->specularSamplerBinding,
            resolveResources->sampler);

        resolveResources->samplerDescriptorSet->Update();

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

        // TODO result
        resolveResources->pipelineState = CreateResolvePipelineState(context, resolveResources.get());

        return { true, std::move(resolveResources) };
    }
}
