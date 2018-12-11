#include <Luch/Render/Deferred/TonemapRenderer.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Deferred/GBufferTextures.h>
#include <Luch/Render/Deferred/TonemapPassResources.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
#include <Luch/Graphics/FrameBuffer.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/SamplerCreateInfo.h>
#include <Luch/Graphics/Swapchain.h>
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

    // Fullscreen quad for triangle list
    // One triangle that covers whole screen
    constexpr Array<QuadVertex, 3> fullscreenQuadVertices =
    {
        QuadVertex { Vec3{-1.0f, -1.0f, 0.0f}, Vec2{0.0f, +1.0f} },
        QuadVertex { Vec3{+3.0f, -1.0f, 0.0f}, Vec2{2.0f, +1.0f} },
        QuadVertex { Vec3{-1.0f, +3.0f, 0.0f}, Vec2{0.0f, -1.0f} },
    };

    const String TonemapRenderer::RendererName{"Tonemap"};

    TonemapRenderer::TonemapRenderer() = default;
    TonemapRenderer::~TonemapRenderer() = default;

    bool TonemapRenderer::Initialize()
    {
        LUCH_ASSERT(context != nullptr);

        auto[tonemapResourcesPrepared, preparedTonemapResources] = PrepareTonemapPassResources(context.get());
        if (!tonemapResourcesPrepared)
        {
            return false;
        }

        resources = std::move(preparedTonemapResources);

        return true;
    }

    bool TonemapRenderer::Deinitialize()
    {
        context.reset();
        resources.reset();

        return true;
    }

    void TonemapRenderer::Tonemap(Texture* input, Texture* output)
    {
        resources->textureDescriptorSet->WriteTexture(resources->colorTextureBinding, input);
        resources->textureDescriptorSet->Update();

        auto [allocateCmdListResult, cmdList] = resources->commandPool->AllocateGraphicsCommandList();
        LUCH_ASSERT(allocateCmdListResult == GraphicsResult::Success);

        FrameBufferCreateInfo frameBufferCreateInfo;
        frameBufferCreateInfo.colorTextures[0] = output;
        frameBufferCreateInfo.renderPass = resources->renderPass;

        auto [createFrameBufferResult, frameBuffer] = context->device->CreateFrameBuffer(frameBufferCreateInfo);
        LUCH_ASSERT(createFrameBufferResult == GraphicsResult::Success);

        int32 framebufferWidth = context->swapchain->GetInfo().width;
        int32 framebufferHeight = context->swapchain->GetInfo().height;

        Viewport viewport {
            0, 0, static_cast<float32>(framebufferWidth), static_cast<float32>(framebufferHeight), 0.0f, 1.0f };
        IntRect scissorRect { {0, 0}, { framebufferWidth, framebufferHeight } };

        cmdList->Begin();
        cmdList->BeginRenderPass(frameBuffer);
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

    RefPtr<PipelineState> TonemapRenderer::CreateTonemapPipelineState(
        RenderContext* context,
        TonemapPassResources* resources)
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
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<TonemapPassResources>> TonemapRenderer::PrepareTonemapPassResources(
        RenderContext* context)
    {
        UniquePtr<TonemapPassResources> tonemapResources = MakeUnique<TonemapPassResources>();

        ColorAttachment colorAttachment;
        colorAttachment.format = context->swapchain->GetInfo().format;
        colorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
        colorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.name =  RendererName;
        renderPassCreateInfo.colorAttachments[0] = colorAttachment;

        auto[createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
        if(createRenderPassResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->renderPass = std::move(createdRenderPass);

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
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->descriptorPool = std::move(createdDescriptorPool);

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return { false };
        }

        tonemapResources->commandPool = std::move(createdCommandPool);

        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32 && LUCH_USE_VULKAN
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\tonemap.vert",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/tonemap_vp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/tonemap.vert",
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

        auto [vertexShaderProgramCreateResult, vertexShaderProgram] = createdVertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex, "vp_main");
        if(vertexShaderProgramCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32 && LUCH_USE_VULKAN
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\tonemap.frag",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/tonemap_fp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/tonemap.frag",
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

        tonemapResources->fragmentShader = std::move(fragmentShaderProgram);

        tonemapResources->colorTextureBinding.OfType(ResourceType::Texture);
        tonemapResources->colorSamplerBinding.OfType(ResourceType::Sampler);

        DescriptorSetLayoutCreateInfo textureDescriptorSetLayoutCreateInfo;
        textureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(1)
            .AddBinding(&tonemapResources->colorTextureBinding);

        auto[createTextureDescriptorSetLayoutResult, createdTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            textureDescriptorSetLayoutCreateInfo);

        if (createTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->textureDescriptorSetLayout = std::move(createdTextureDescriptorSetLayout);

        DescriptorSetLayoutCreateInfo samplerDescriptorSetLayoutCreateInfo;
        samplerDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Sampler)
            .WithNBindings(1)
            .AddBinding(&tonemapResources->colorSamplerBinding);

        auto[createSamplerDescriptorSetLayoutResult, createdSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
            samplerDescriptorSetLayoutCreateInfo);

        if (createSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->samplerDescriptorSetLayout = std::move(createdSamplerDescriptorSetLayout);

        SamplerCreateInfo samplerCreateInfo;

        auto[createSamplerResult, createdSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createSamplerResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->sampler = std::move(createdSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, tonemapResources->textureDescriptorSetLayout)
            .AddSetLayout(ShaderStage::Fragment, tonemapResources->samplerDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->pipelineLayout = std::move(createdPipelineLayout);

        BufferCreateInfo quadBufferCreateInfo;
        quadBufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
        // TODO
        quadBufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        quadBufferCreateInfo.usage = BufferUsageFlags::VertexBuffer;

        auto[createQuadBufferResult, createdQuadBuffer] = context->device->CreateBuffer(
            quadBufferCreateInfo,
            fullscreenQuadVertices.data());

        if (createQuadBufferResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->fullscreenQuadBuffer = std::move(createdQuadBuffer);

        auto [allocateTextureSetResult, allocatedTextureSet] = tonemapResources->descriptorPool->AllocateDescriptorSet(
            tonemapResources->textureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        tonemapResources->textureDescriptorSet = std::move(allocatedTextureSet);

        auto [allocateSamplerSetResult, allocatedSamplerSet] = tonemapResources->descriptorPool->AllocateDescriptorSet(
            tonemapResources->samplerDescriptorSetLayout);

        if (allocateSamplerSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        tonemapResources->samplerDescriptorSet = std::move(allocatedSamplerSet);

        tonemapResources->samplerDescriptorSet->WriteSampler(
            tonemapResources->colorSamplerBinding,
            tonemapResources->sampler);

        tonemapResources->samplerDescriptorSet->Update();

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedUniformBufferSize;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createBufferResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        tonemapResources->sharedBuffer = MakeUnique<SharedBuffer>(std::move(createdBuffer));

        // TODO result
        tonemapResources->pipelineState = CreateTonemapPipelineState(context, tonemapResources.get());

        return { true, std::move(tonemapResources) };
    }
}
