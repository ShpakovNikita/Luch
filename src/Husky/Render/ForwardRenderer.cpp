#include <Husky/Render/ForwardRenderer.h>
#include <Husky/PrimitiveTopology.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/AlphaMode.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/AttributeSemantic.h>

#include <Husky/Vulkan/Attachment.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/CommandBuffer.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/DescriptorSetBinding.h>
#include <Husky/Vulkan/RenderPassCreateInfo.h>
#include <Husky/Vulkan/SubpassDescription.h>
#include <Husky/Vulkan/VertexBufferObject.h>
#include <Husky/Vulkan/IndexBufferObject.h>
#include <Husky/Vulkan/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Vulkan/PipelineLayoutCreateInfo.h>
#include <Husky/Vulkan/FramebufferCreateInfo.h>
#include <Husky/Vulkan/IndexType.h>
#include <Husky/Vulkan/PipelineCreateInfo.h>

namespace Husky::Render
{
    using namespace Vulkan;

    static const Map<SceneV1::AttributeSemantic, int32> SemanticToLocation =
    {
        { SceneV1::AttributeSemantic::Position, 0 },
        { SceneV1::AttributeSemantic::Normal, 1 },
        { SceneV1::AttributeSemantic::Tangent, 2 },
        { SceneV1::AttributeSemantic::Texcoord_0, 3 },
        { SceneV1::AttributeSemantic::Texcoord_1, 4 },
        { SceneV1::AttributeSemantic::Color_0, 5 },
    };

    static const char8* vertexShaderSource =
        "#version 450\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "layout (set = 0, binding = 0) uniform UniformBufferObject\n"
        "{\n"
        "    mat4 model;\n"
        "    mat4 view;\n"
        "    mat4 projection;\n"
        "} ub;\n"
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec3 normal;\n"
        "layout (location = 2) in vec2 inTexCoord;\n"
        "layout (location = 0) out vec2 outTexCoord;\n"
        "void main()\n"
        "{\n"
        "   outTexCoord = inTexCoord;\n"
        "   gl_Position = ub.projection * ub.view * ub.model * vec4(position, 1.0);\n"
        "}\n";

    static const char8* fragmentShaderSource =
        "#version 450\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "layout (location = 0) in vec2 texCoord;\n"
        "layout (location = 0) out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "   outColor = vec4(texCoord, 0.0, 1.0);\n"
        "}\n";

    ForwardRenderer::ForwardRenderer(
        PhysicalDevice* physicalDevice,
        Surface* surface,
        int32 aWidth,
        int32 aHeight)
        : width(aWidth)
        , height(aHeight)
    {
        context = MakeUnique<ForwardRendererContext>();
        context->physicalDevice = physicalDevice;
        context->surface = surface;
    }

    bool ForwardRenderer::Initialize()
    {
        GLSLShaderCompiler::Initialize();

        auto[chooseQueuesResult, queueIndices] = context->physicalDevice->ChooseDeviceQueues(context->surface.Get());
        if (chooseQueuesResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        auto[createDeviceResult, createdDevice] = context->physicalDevice->CreateDevice(std::move(queueIndices), GetRequiredDeviceExtensionNames());
        if (createDeviceResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        context->device = std::move(createdDevice);

        auto& device = context->device;
        auto indices = context->device->GetQueueIndices();

        auto[createdPresentCommandPoolResult, createdPresentCommandPool] = device->CreateCommandPool(indices->presentQueueFamilyIndex, true, false);
        if (createdPresentCommandPoolResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        context->presentCommandPool = std::move(createdPresentCommandPool);

        auto[swapchainChooseCreateInfoResult, swapchainCreateInfo] = Swapchain::ChooseSwapchainCreateInfo(width, height, context->physicalDevice.Get(), context->surface.Get());
        if (swapchainChooseCreateInfoResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        auto[createSwapchainResult, createdSwapchain] = device->CreateSwapchain(swapchainCreateInfo, context->surface.Get());
        if (createSwapchainResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        context->swapchain = std::move(createdSwapchain);

        constexpr auto uniformBufferSize = sizeof(Uniform);
        
        auto vertexShaderCompiled = context->shaderCompiler.TryCompileShader(ShaderStage::Vertex, vertexShaderSource, context->vertexShaderBytecode);
        HUSKY_ASSERT(vertexShaderCompiled, "Vertex shader failed to compile");

        auto fragmentShaderCompiled = context->shaderCompiler.TryCompileShader(ShaderStage::Fragment, fragmentShaderSource, context->fragmentShaderBytecode);
        HUSKY_ASSERT(fragmentShaderCompiled, "Fragment shader failed to compile");

        auto[createVertexShaderModuleResult, createdVertexShaderModule] = device->CreateShaderModule(
            context->vertexShaderBytecode.data(),
            context->vertexShaderBytecode.size() * sizeof(uint32)
        );

        if (createVertexShaderModuleResult != vk::Result::eSuccess)
        {
            return false;
        }

        context->vertexShaderModule = std::move(createdVertexShaderModule);

        auto[createFragmentShaderModuleResult, createdFragmentShaderModule] = device->CreateShaderModule(
            context->fragmentShaderBytecode.data(),
            context->fragmentShaderBytecode.size() * sizeof(uint32));

        if (createFragmentShaderModuleResult != vk::Result::eSuccess)
        {
            return false;
        }

        context->fragmentShaderModule = std::move(createdFragmentShaderModule);

        Attachment colorAttachment;
        colorAttachment
            .SetFormat(context->swapchain->GetFormat())
            .SetInitialLayout(vk::ImageLayout::eUndefined)
            .SetLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStoreOp(vk::AttachmentStoreOp::eStore)
            .SetSampleCount(SampleCount::e1)
            .SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        Attachment depthAttachment;
        depthAttachment
            .SetFormat(depthStencilFormat)
            .SetInitialLayout(vk::ImageLayout::eUndefined)
            .SetLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStoreOp(vk::AttachmentStoreOp::eStore)
            .SetStencilLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .SetSampleCount(SampleCount::e1)
            .SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        SubpassDescription subpass;
        subpass
            .AddColorAttachment(&colorAttachment, vk::ImageLayout::eColorAttachmentOptimal)
            .WithDepthStencilAttachment(&depthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
            .AddAttachment(&colorAttachment)
            .AddAttachment(&depthAttachment)
            .AddSubpass(std::move(subpass));

        auto[createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
        if (createRenderPassResult != vk::Result::eSuccess)
        {
            return false;
        }

        context->renderPass = std::move(createdRenderPass);

        DescriptorSetBinding uniformBufferBinding;
        uniformBufferBinding
            .OfType(vk::DescriptorType::eUniformBuffer)
            .AtStages(ShaderStage::Vertex);

        DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo
            .AddBinding(&uniformBufferBinding);

        auto[createDescriptorSetLayoutResult, createdDescriptorSetLayout] = device->CreateDescriptorSetLayout(descriptorSetLayoutCreateInfo);
        if (createDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return false;
        }

        context->descriptorSetLayout = std::move(createdDescriptorSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .WithNSetLayouts(1)
            .AddSetLayout(context->descriptorSetLayout.Get());

        auto[createPipelineLayoutResult, createdPipelineLayout] = device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != vk::Result::eSuccess)
        {
            return false;
        }

        context->pipelineLayout = std::move(createdPipelineLayout);

        context->frameResources.reserve(swapchainCreateInfo.imageCount);

        for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
        {
            auto& frameResources = context->frameResources.emplace_back();

            auto[createdGraphicsCommandPoolResult, createdGraphicsCommandPool] = device->CreateCommandPool(indices->graphicsQueueFamilyIndex, false, false);
            if (createdGraphicsCommandPoolResult != vk::Result::eSuccess)
            {
                // TODO
                return false;
            }

            frameResources.graphicsCommandPool = std::move(createdGraphicsCommandPool);

            //auto[createBufferResult, createdBuffer] = device->CreateBuffer(uniformBufferSize, indices->graphicsQueueFamilyIndex, vk::BufferUsageFlagBits::eUniformBuffer, true);
            //if (createBufferResult != vk::Result::eSuccess)
            //{
            //    // TODO
            //    return false;
            //}
            //
            //frameResources.uniformBuffer = std::move(createdBuffer);

            //auto[createVertexBufferResult, createdVertexBuffer] = device.CreateVertexBuffer(vertexSize, verticesCount, indices.graphicsQueueFamilyIndex, true);
            //if (createVertexBufferResult != vk::Result::eSuccess)
            //{
            //    return false;
            //}

            //{
            //    auto[mapVerticesResult, verticesMemory] = createdVertexBuffer.GetUnderlyingBuffer().MapMemory(indexBufferSize, 0);
            //    if (mapVerticesResult != vk::Result::eSuccess)
            //    {
            //        return false;
            //    }

            //    memcpy(verticesMemory, graphicsContext->boxData.vertices.data(), vertexSize * verticesCount);

            //    createdVertexBuffer.GetUnderlyingBuffer().UnmapMemory();
            //}

            //frameResources.vertexBuffer = std::move(createdVertexBuffer);

            //auto[createIndexBufferResult, createdIndexBuffer] = device.CreateIndexBuffer(IndexType::UInt16, indicesCount, indices.graphicsQueueFamilyIndex, true);
            //if (createIndexBufferResult != vk::Result::eSuccess)
            //{
            //    // TODO
            //    return false;
            //}

            //{
            //    auto[mapIndicesResult, indicesMemory] = createdIndexBuffer.GetUnderlyingBuffer().MapMemory(indexBufferSize, 0);
            //    if (mapIndicesResult != vk::Result::eSuccess)
            //    {
            //        return false;
            //    }
            //    memcpy(indicesMemory, graphicsContext->boxData.indices16.data(), indexBufferSize);

            //    createdIndexBuffer.GetUnderlyingBuffer().UnmapMemory();
            //}

            //frameResources.indexBuffer = std::move(createdIndexBuffer);

            vk::ImageCreateInfo depthBufferCreateInfo;
            depthBufferCreateInfo.setFormat(ToVulkanFormat(depthStencilFormat));
            depthBufferCreateInfo.setArrayLayers(1);
            depthBufferCreateInfo.setImageType(vk::ImageType::e2D);
            depthBufferCreateInfo.setExtent({ (uint32)swapchainCreateInfo.width, (uint32)swapchainCreateInfo.height, 1 });
            depthBufferCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
            depthBufferCreateInfo.setMipLevels(1);
            depthBufferCreateInfo.setQueueFamilyIndexCount(1);
            depthBufferCreateInfo.setPQueueFamilyIndices(&indices->graphicsQueueFamilyIndex);
            depthBufferCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
            depthBufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
            depthBufferCreateInfo.setTiling(vk::ImageTiling::eOptimal);
            depthBufferCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);

            auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = device->CreateImage(depthBufferCreateInfo);
            if (createDepthStencilBufferResult != vk::Result::eSuccess)
            {
                return false;
            }

            frameResources.depthStencilBuffer = std::move(createdDepthStencilBuffer);

            auto[createDepthStencilBufferViewResult, createdDepthStencilBufferView] = device->CreateImageView(frameResources.depthStencilBuffer.Get());
            if (createDepthStencilBufferViewResult != vk::Result::eSuccess)
            {
                return false;
            }

            frameResources.depthStencilBufferView = std::move(createdDepthStencilBufferView);

            FramebufferCreateInfo framebufferCreateInfo(context->renderPass.Get(), swapchainCreateInfo.width, swapchainCreateInfo.height, 1);
            framebufferCreateInfo
                .AddAtachment(&colorAttachment, context->swapchain->GetImageView(i))
                .AddAtachment(&depthAttachment, frameResources.depthStencilBufferView.Get());

            auto[createFramebufferResult, createdFramebuffer] = device->CreateFramebuffer(framebufferCreateInfo);
            if (createFramebufferResult != vk::Result::eSuccess)
            {
                return false;
            }

            frameResources.framebuffer = std::move(createdFramebuffer);

            auto[createFenceResult, createdFence] = device->CreateFence();
            if (createFenceResult != vk::Result::eSuccess)
            {
                return false;
            }

            auto[allocateResult, allocatedBuffer] = frameResources.graphicsCommandPool->AllocateCommandBuffer(CommandBufferLevel::Primary);
            if (allocateResult != vk::Result::eSuccess)
            {
                return false;
            }

            frameResources.commandBuffer = std::move(allocatedBuffer);

            frameResources.fence = std::move(createdFence);

            auto[createSemaphoreResult, createdSemaphore] = device->CreateSemaphore();
            if (createSemaphoreResult != vk::Result::eSuccess)
            {
                return false;
            }

            frameResources.semaphore = std::move(createdSemaphore);

            //auto[createDescriptorPoolResult, createdDescriptorPool] = device->CreateDescriptorPool(1, { { vk::DescriptorType::eUniformBuffer, 1 } });
            //if (createDescriptorPoolResult != vk::Result::eSuccess)
            //{
            //    return false;
            //}
            //
            //frameResources.descriptorPool = std::move(createdDescriptorPool);
            //
            //{
            //    auto[mapUniformResult, uniformMemory] = frameResources.uniformBuffer.MapMemory(uniformBufferSize, 0);
            //    if (mapUniformResult != vk::Result::eSuccess)
            //    {
            //        return false;
            //    }
            //
            //
            //    // TODO
            //    memcpy(uniformMemory, nullptr, uniformBufferSize);
            //
            //    frameResources.uniformBuffer.UnmapMemory();
            //}
        }
    }

    bool ForwardRenderer::Deinitialize()
    {
        context.release();
        GLSLShaderCompiler::Deinitialize();
        return true;
    }

    void ForwardRenderer::Draw(const RefPtr<SceneV1::Node>& node)
    {
        auto[createSemaphoreResult, imageAcquiredSemaphore] = context->device->CreateSemaphore();
        HUSKY_ASSERT(createSemaphoreResult == vk::Result::eSuccess);

        auto[acquireResult, index] = context->swapchain->AcquireNextImage(nullptr, imageAcquiredSemaphore.Get());
        HUSKY_ASSERT(acquireResult == vk::Result::eSuccess);

        auto& frameResource = context->frameResources[index];

        //frameResource.descriptorPool->Reset();

        //auto[allocateDescriptorSetResult, allocatedDescriptorSet] = frameResource.descriptorPool.AllocateDescriptorSet(&context->descriptorSetLayout);
        //HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);
        //
        //frameResource.descriptorSet = std::move(allocatedDescriptorSet);

        // DescriptorSetWrites descriptorSetWrites;
        // descriptorSetWrites.WriteUniformBufferDescriptors(&frameResource.descriptorSet, &context->uniformBufferBinding, { &frameResource.uniformBuffer });
        // 
        // DescriptorSet::Update(descriptorSetWrites);

        auto &cmdBuffer = frameResource.commandBuffer;

        Array<float32, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        vk::ClearColorValue colorClearValue{ clearColor };
        vk::ClearDepthStencilValue depthStencilClearValue{ maxDepth, 0 };

        Vector<vk::ClearValue> clearValues = { colorClearValue, depthStencilClearValue };

        int32 framebufferWidth = context->swapchain->GetSwapchainCreateInfo().width;
        int32 framebufferHeight = context->swapchain->GetSwapchainCreateInfo().height;

        cmdBuffer
            ->Begin()
            ->BeginInlineRenderPass(&context->renderPass, &frameResource.framebuffer, clearValues, { { 0, 0 },{ (uint32)framebufferWidth, (uint32)framebufferHeight } });

        cmdBuffer
            ->EndRenderPass()
            ->End();

        Submission submission;
        submission.commandBuffers = { cmdBuffer.Get() };
        submission.fence = frameResource.fence.Get();
        submission.waitOperations = { { imageAcquiredSemaphore.Get(), vk::PipelineStageFlagBits::eColorAttachmentOutput } };
        submission.signalSemaphores = { frameResource.semaphore.Get() };

        context->device->GetGraphicsQueue()->Submit(submission);

        frameResource.fence->Wait();
        frameResource.fence->Reset();

        frameResource.graphicsCommandPool->Reset();

        PresentSubmission presentSubmission;
        presentSubmission.index = index;
        presentSubmission.swapchain = context->swapchain.Get();
        presentSubmission.waitSemaphores = { frameResource.semaphore.Get() };

        context->device->GetPresentQueue()->Present(presentSubmission);
    }

    void ForwardRenderer::PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, GlobalDrawContext * drawContext)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            RefPtr<Pipeline> pipeline = primitive->GetPipeline();
            if (pipeline == nullptr)
            {
                pipeline = CreatePipeline(primitive);
                primitive->SetPipeline(pipeline);
            }

            const auto& indexBuffer = primitive->GetIndexBuffer();
            const auto& vertexBuffers = primitive->GetVertexBuffers();
            const auto& material = primitive->GetMaterial();

            //indexBuffer.
        }
    }

    void ForwardRenderer::DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, GlobalDrawContext* drawContext, CommandBuffer* cmdBuffer)
    {
        for(const auto& primitive : mesh->GetPrimitives())
        {
            if (primitive->GetMaterial()->alphaMode != SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, drawContext, cmdBuffer);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            if (primitive->GetMaterial()->alphaMode == SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, drawContext, cmdBuffer);
            }
        }
    }

    void ForwardRenderer::DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, GlobalDrawContext* drawContext, CommandBuffer* cmdBuffer)
    {
        RefPtr<Pipeline> pipeline = primitive->GetPipeline();

        const auto& vertexBuffers = primitive->GetVertexBuffers();

        Vector<Buffer*> vulkanVertexBuffers;
        Vector<int64> offsets;
        vulkanVertexBuffers.reserve(vertexBuffers.size());
        offsets.reserve(vertexBuffers.size());

        for (const auto& vertexBuffer : vertexBuffers)
        {
            vulkanVertexBuffers.push_back(vertexBuffer->GetDeviceBuffer()->GetUnderlyingBuffer());
            offsets.push_back(vertexBuffer->GetByteOffset());
        }

        const auto& indexBuffer = primitive->GetIndexBuffer();

        cmdBuffer
            ->BindGraphicsPipeline(pipeline.Get())
            ->BindVertexBuffers(vulkanVertexBuffers, offsets, 0)
            // TODO push material constants
            ->BindIndexBuffer(
                indexBuffer->GetDeviceBuffer()->GetUnderlyingBuffer(),
                ToVulkanIndexType(indexBuffer->GetIndexType()),
                indexBuffer->GetByteOffset())
            ->DrawIndexed(indexBuffer->GetIndexCount(), 1, 0, 0, 0);
    }

    // Assume that all primitives have the same vertex layout
    RefPtr<Pipeline> ForwardRenderer::CreatePipeline(const RefPtr<SceneV1::Primitive>& primitive)
    {
        GraphicsPipelineCreateInfo pipelineState;

        auto& vertexShaderStage = pipelineState.shaderStages.emplace_back();
        vertexShaderStage.name = "main";
        vertexShaderStage.shaderModule = context->vertexShaderModule.Get();
        vertexShaderStage.stage = ShaderStage::Vertex;

        auto& fragmentShaderStage = pipelineState.shaderStages.emplace_back();
        fragmentShaderStage.name = "main";
        fragmentShaderStage.shaderModule = context->fragmentShaderModule.Get();
        fragmentShaderStage.stage = ShaderStage::Fragment;

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        pipelineState.vertexInputState.bindingDescriptions.resize(vertexBuffers.size());
        for (int i = 0; i < vertexBuffers.size(); i++)
        {
            const auto& vertexBuffer = vertexBuffers[i];
            auto& bindingDescription = pipelineState.vertexInputState.bindingDescriptions[i];
            bindingDescription.binding = i;
            bindingDescription.stride = vertexBuffer->GetStride();
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        }

        const auto& attributes = primitive->GetAttributes();
        pipelineState.vertexInputState.attributeDescriptions.reserve(attributes.size());
        for (const auto& attribute : attributes)
        {
            auto& attributeDescription = pipelineState.vertexInputState.attributeDescriptions.emplace_back();
            attributeDescription.location = SemanticToLocation.at(attribute.semantic);
            attributeDescription.binding = attribute.vertexBufferIndex;
            attributeDescription.format = ToVulkanFormat(attribute.format);
            attributeDescription.offset = attribute.offset;
        }

        // TODO
        pipelineState.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

        const auto& material = primitive->GetMaterial();

        if (material->doubleSided)
        {
            pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
        }
        else
        {
            pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
        }

        pipelineState.depthStencilState.depthTestEnable = true;
        pipelineState.depthStencilState.depthWriteEnable = true;
        pipelineState.depthStencilState.depthCompareOp = vk::CompareOp::eLess;

        auto &attachmentBlendState = pipelineState.colorBlendState.attachments.emplace_back();
        attachmentBlendState.blendEnable = material->alphaMode == SceneV1::AlphaMode::Blend;
        if (attachmentBlendState.blendEnable)
        {
            attachmentBlendState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            attachmentBlendState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            attachmentBlendState.colorBlendOp = vk::BlendOp::eAdd;
            attachmentBlendState.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            attachmentBlendState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            attachmentBlendState.alphaBlendOp = vk::BlendOp::eAdd;
        }

        pipelineState.renderPass = context->renderPass.Get();
        pipelineState.layout = context->pipelineLayout.Get();

        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eScissor);
        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eViewport);

        auto[createPipelineResult, createdPipeline] = context->device->CreateGraphicsPipeline(pipelineState);
        if (createPipelineResult != vk::Result::eSuccess)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    Vector<const char8*> ForwardRenderer::GetRequiredDeviceExtensionNames() const
    {
        Vector<const char8*> requiredExtensionNames;

        requiredExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        return requiredExtensionNames;
    }
}
