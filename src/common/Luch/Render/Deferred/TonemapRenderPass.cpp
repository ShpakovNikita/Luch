#include <Luch/Render/Deferred/TonemapRenderPass.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Deferred/TonemapContext.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>

#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/PipelineState.h>
#include <Luch/Graphics/RenderPass.h>
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
    // One triangle that covers whole screen
    constexpr Array<QuadVertex, 3> fullscreenQuadVertices =
    {
        QuadVertex { Vec3{-1.0f, -1.0f, 0.0f}, Vec2{0.0f, +1.0f} },
        QuadVertex { Vec3{+3.0f, -1.0f, 0.0f}, Vec2{2.0f, +1.0f} },
        QuadVertex { Vec3{-1.0f, +3.0f, 0.0f}, Vec2{0.0f, -1.0f} },
    };

    const String TonemapRenderPass::RenderPassName{"Tonemap"};

    TonemapRenderPass::TonemapRenderPass(
        TonemapPersistentContext* aPersistentContext,
        TonemapTransientContext* aTransientContext,
        RenderGraphBuilder* builder)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        auto node = builder->AddRenderPass(RenderPassName, persistentContext->renderPass, this);

        input = node->ReadsTexture(transientContext->inputHandle);

        output = node->CreateColorAttachment(0, transientContext->attachmentSize);
    }

    TonemapRenderPass::~TonemapRenderPass() = default;

    void TonemapRenderPass::PrepareScene()
    {
    }

    void TonemapRenderPass::UpdateScene()
    {
    }

    void TonemapRenderPass::ExecuteRenderPass(
        RenderGraphResourceManager* manager,
        FrameBuffer* frameBuffer,
        GraphicsCommandList* cmdList)
    {
        auto hdrTexture = manager->GetTexture(input);

        transientContext->textureDescriptorSet->WriteTexture(persistentContext->colorTextureBinding, hdrTexture);
        transientContext->textureDescriptorSet->Update();

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
            transientContext->textureDescriptorSet);

        cmdList->BindVertexBuffers({ persistentContext->fullscreenQuadBuffer }, {0}, 0);
        cmdList->Draw(0, fullscreenQuadVertices.size());
        cmdList->EndRenderPass();
        cmdList->End();
    }

    RefPtr<PipelineState> TonemapRenderPass::CreateTonemapPipelineState(
        TonemapPersistentContext* persistentContext)
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
        colorAttachment.format = persistentContext->renderPass->GetCreateInfo().colorAttachments[0]->format;
        colorAttachment.blendEnable = false;

        ci.depthStencil.depthStencilFormat = Format::Undefined;

        //pipelineState.renderPass = lighting.renderPass;
        ci.pipelineLayout = persistentContext->pipelineLayout;
        ci.vertexProgram = persistentContext->vertexShader;
        ci.fragmentProgram = persistentContext->fragmentShader;

        auto[createPipelineResult, createdPipeline] = persistentContext->device->CreatePipelineState(ci);
        if (createPipelineResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
        }

        return createdPipeline;
    }

    ResultValue<bool, UniquePtr<TonemapPersistentContext>> TonemapRenderPass::PrepareTonemapPersistentContext(
        GraphicsDevice* device,
        Format swapchainFormat)
    {
        UniquePtr<TonemapPersistentContext> context = MakeUnique<TonemapPersistentContext>();
        context->device = device;

        ColorAttachment colorAttachment;
        colorAttachment.format = swapchainFormat;
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
            device,
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

        context->vertexShader = std::move(vertexShaderProgram);

        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
            device,
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

        context->fragmentShader = std::move(fragmentShaderProgram);

        context->colorTextureBinding.OfType(ResourceType::Texture);

        DescriptorSetLayoutCreateInfo textureDescriptorSetLayoutCreateInfo;
        textureDescriptorSetLayoutCreateInfo
            .OfType(DescriptorSetType::Texture)
            .WithNBindings(1)
            .AddBinding(&context->colorTextureBinding);

        auto[createTextureDescriptorSetLayoutResult, createdTextureDescriptorSetLayout] = device->CreateDescriptorSetLayout(
            textureDescriptorSetLayoutCreateInfo);

        if (createTextureDescriptorSetLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->textureDescriptorSetLayout = std::move(createdTextureDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Fragment, context->textureDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        context->pipelineLayout = std::move(createdPipelineLayout);

        BufferCreateInfo quadBufferCreateInfo;
        quadBufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
        // TODO
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

        context->pipelineState = CreateTonemapPipelineState(context.get());

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<TonemapTransientContext>> TonemapRenderPass::PrepareTonemapTransientContext(
        TonemapPersistentContext* persistentContext,
        RefPtr<DescriptorPool> descriptorPool)
    {
        auto context = MakeUnique<TonemapTransientContext>();
        context->descriptorPool = descriptorPool;

        auto [allocateTextureSetResult, allocatedTextureSet] = context->descriptorPool->AllocateDescriptorSet(
                persistentContext->textureDescriptorSetLayout);

        if (allocateTextureSetResult != GraphicsResult::Success)
        {
            return { false };
        }

        context->textureDescriptorSet = std::move(allocatedTextureSet);

        return { true, std::move(context) };
    }
}