#include <Husky/Render/ForwardRenderer.h>
#include <Husky/Render/TextureUploader.h>

#include <Husky/PrimitiveTopology.h>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/AlphaMode.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/AttributeSemantic.h>

#include <Husky/Vulkan/Attachment.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/DescriptorSetWrites.h>
#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/DescriptorPool.h>
#include <Husky/Vulkan/CommandBuffer.h>
#include <Husky/Vulkan/Swapchain.h>
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/DescriptorSetBinding.h>
#include <Husky/Vulkan/RenderPassCreateInfo.h>
#include <Husky/Vulkan/SubpassDescription.h>
#include <Husky/Vulkan/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Vulkan/PipelineLayoutCreateInfo.h>
#include <Husky/Vulkan/FramebufferCreateInfo.h>
#include <Husky/Vulkan/IndexType.h>
#include <Husky/Vulkan/PipelineCreateInfo.h>
#include <Husky/Vulkan/Fence.h>

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
    }

    bool ForwardRenderer::Deinitialize()
    {
        context.release();
        GLSLShaderCompiler::Deinitialize();
        return true;
    }

    void ForwardRenderer::DrawScene(const RefPtr<SceneV1::Scene>& scene)
    {

    }

    ResultValue<bool, PreparedScene> ForwardRenderer::PrepareScene(const RefPtr<SceneV1::Scene>& scene)
    {
        PreparedScene preparedScene;

        const auto& sceneProperties = scene->GetSceneProperties();

        const int32 texturesPerMaterial = 5;
        const int32 samplersPerMaterial = 5;

        int32 textureDescriptorCount = sceneProperties.materials.size() * texturesPerMaterial;
        int32 samplerDescriptorCount = sceneProperties.materials.size() * samplersPerMaterial;
        int32 perMeshUBOCount = sceneProperties.meshes.size();
        int32 perCameraUBOCount = 1;

        // one set per material, one set per mesh, one set for camera
        int32 maxDescriptorSets = sceneProperties.materials.size() + sceneProperties.meshes.size() + 1;

        UnorderedMap<vk::DescriptorType, int32> descriptorCount = 
        {
            { vk::DescriptorType::eSampledImage, textureDescriptorCount},
            { vk::DescriptorType::eSampler, samplerDescriptorCount },
            { vk::DescriptorType::eUniformBuffer, perMeshUBOCount + perCameraUBOCount}
        };

        auto vertexShaderCompiled = context->shaderCompiler.TryCompileShader(ShaderStage::Vertex, vertexShaderSource, preparedScene.vertexShaderBytecode);
        HUSKY_ASSERT(vertexShaderCompiled, "Vertex shader failed to compile");

        auto fragmentShaderCompiled = context->shaderCompiler.TryCompileShader(ShaderStage::Fragment, fragmentShaderSource, preparedScene.fragmentShaderBytecode);
        HUSKY_ASSERT(fragmentShaderCompiled, "Fragment shader failed to compile");

        auto[createVertexShaderModuleResult, createdVertexShaderModule] = context->device->CreateShaderModule(
            preparedScene.vertexShaderBytecode.data(),
            preparedScene.vertexShaderBytecode.size() * sizeof(uint32)
        );

        if (createVertexShaderModuleResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.vertexShaderModule = std::move(createdVertexShaderModule);

        auto[createFragmentShaderModuleResult, createdFragmentShaderModule] = context->device->CreateShaderModule(
            preparedScene.fragmentShaderBytecode.data(),
            preparedScene.fragmentShaderBytecode.size() * sizeof(uint32));

        if (createFragmentShaderModuleResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.fragmentShaderModule = std::move(createdFragmentShaderModule);

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

        auto[createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
        if (createRenderPassResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.renderPass = std::move(createdRenderPass);

        preparedScene.cameraUniformBufferBinding
            .OfType(vk::DescriptorType::eUniformBuffer)
            .AtStages(ShaderStage::Vertex | ShaderStage::Fragment);

        preparedScene.meshUniformBufferBinding
            .OfType(vk::DescriptorType::eUniformBuffer)
            .AtStages(ShaderStage::Vertex);

        preparedScene.materialImageBinding
            .OfType(vk::DescriptorType::eSampledImage)
            .AtStages(ShaderStage::Fragment)
            .WithNBindings(texturesPerMaterial);

        preparedScene.materialSamplerBinding
            .OfType(vk::DescriptorType::eSampler)
            .AtStages(ShaderStage::Fragment)
            .WithNBindings(samplersPerMaterial);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .AddBinding(&preparedScene.cameraUniformBufferBinding);

        DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
        meshDescriptorSetLayoutCreateInfo
            .AddBinding(&preparedScene.meshUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialDescriptorSetLayoutCreateInfo;
        materialDescriptorSetLayoutCreateInfo
            .AddBinding(&preparedScene.materialImageBinding)
            .AddBinding(&preparedScene.materialSamplerBinding);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cameraDescriptorSetLayoutCreateInfo);
        if (createCameraDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.cameraDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(meshDescriptorSetLayoutCreateInfo);
        if (createMeshDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.meshDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);

        
        auto[createMaterialDescriptorSetLayoutResult, createdMaterialDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(materialDescriptorSetLayoutCreateInfo);
        if (createMaterialDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.materialDescriptorSetLayout = std::move(createdMaterialDescriptorSetLayout);


        // TODO check push constant size limit

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .WithNSetLayouts(3)
            .AddSetLayout(preparedScene.cameraDescriptorSetLayout)
            .AddSetLayout(preparedScene.meshDescriptorSetLayout)
            .AddSetLayout(preparedScene.materialDescriptorSetLayout)
            .WithNPushConstantRanges(1)
            .AddPushConstantRange({ ShaderStage::Fragment, 0, sizeof(MaterialPushConstants) });

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.pipelineLayout = std::move(createdPipelineLayout);

        const auto& swapchainCreateInfo = context->swapchain->GetSwapchainCreateInfo();

        preparedScene.frameResources.reserve(swapchainCreateInfo.imageCount);

        const auto& indices = context->device->GetQueueIndices();

        auto[createdGraphicsCommandPoolResult, createdGraphicsCommandPool] = context->device->CreateCommandPool(indices->graphicsQueueFamilyIndex, false, false);
        if (createdGraphicsCommandPoolResult != vk::Result::eSuccess)
        {
            // TODO
            return { false };
        }

        preparedScene.commandPool = std::move(createdGraphicsCommandPool);

        for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
        {
            auto& frameResources = preparedScene.frameResources.emplace_back();

            auto[createFrameGraphicsCommandPoolResult, createdFrameGraphicsCommandPool] = context->device->CreateCommandPool(indices->graphicsQueueFamilyIndex, false, false);
            if (createFrameGraphicsCommandPoolResult != vk::Result::eSuccess)
            {
                // TODO
                return { false };
            }

            frameResources.graphicsCommandPool = std::move(createdFrameGraphicsCommandPool);

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

            auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = context->device->CreateImage(depthBufferCreateInfo);
            if (createDepthStencilBufferResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.depthStencilBuffer = std::move(createdDepthStencilBuffer);

            auto[createDepthStencilBufferViewResult, createdDepthStencilBufferView] = context->device->CreateImageView(frameResources.depthStencilBuffer.Get());
            if (createDepthStencilBufferViewResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.depthStencilBufferView = std::move(createdDepthStencilBufferView);

            FramebufferCreateInfo framebufferCreateInfo(preparedScene.renderPass.Get(), swapchainCreateInfo.width, swapchainCreateInfo.height, 1);
            framebufferCreateInfo
                .AddAtachment(&colorAttachment, context->swapchain->GetImageView(i))
                .AddAtachment(&depthAttachment, frameResources.depthStencilBufferView.Get());

            auto[createFramebufferResult, createdFramebuffer] = context->device->CreateFramebuffer(framebufferCreateInfo);
            if (createFramebufferResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.framebuffer = std::move(createdFramebuffer);

            auto[createFenceResult, createdFence] = context->device->CreateFence();
            if (createFenceResult != vk::Result::eSuccess)
            {
                return { false };
            }

            auto[allocateResult, allocatedBuffer] = frameResources.graphicsCommandPool->AllocateCommandBuffer(CommandBufferLevel::Primary);
            if (allocateResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.commandBuffer = std::move(allocatedBuffer);

            frameResources.fence = std::move(createdFence);

            auto[createSemaphoreResult, createdSemaphore] = context->device->CreateSemaphore();
            if (createSemaphoreResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.semaphore = std::move(createdSemaphore);

            auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(maxDescriptorSets, descriptorCount);
            if (createDescriptorPoolResult != vk::Result::eSuccess)
            {
                return { false };
            }
            
            preparedScene.descriptorPool = std::move(createdDescriptorPool);

            auto[createCameraDescriptorSetResult , createdCameraDescriptorSet] = preparedScene.descriptorPool->AllocateDescriptorSet(preparedScene.cameraDescriptorSetLayout);
            if (createCameraDescriptorSetResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.cameraDescriptorSet = std::move(createdCameraDescriptorSet);

            auto[createUniformBufferResult, createdUniformBuffer] = context->device->CreateBuffer(
                sizeof(CameraUniformBuffer),
                indices->graphicsQueueFamilyIndex,
                vk::BufferUsageFlagBits::eUniformBuffer,
                true);

            if (createUniformBufferResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.cameraUniformBuffer = std::move(createdUniformBuffer);

            DescriptorSetWrites descriptorSetWrites;
            descriptorSetWrites.WriteUniformBufferDescriptors(frameResources.cameraDescriptorSet, &preparedScene.cameraUniformBufferBinding, { frameResources.cameraUniformBuffer });
            
            DescriptorSet::Update(descriptorSetWrites);
        }

        const auto& textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            texturesVector.push_back(texture);
        }

        TextureUploader textureUploader{ context->device, preparedScene.commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);

        HUSKY_ASSERT(uploadTexturesSucceeded);

        // TODO semaphores

        Submission uploadTexturesSubmission;
        for (const auto& buffer : uploadTexturesResult.commandBuffers)
        {
            uploadTexturesSubmission.commandBuffers.push_back(buffer);
        }

        context->device->GetGraphicsQueue()->Submit(uploadTexturesSubmission);
        context->device->GetGraphicsQueue()->WaitIdle();

        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            PrepareNode(node, preparedScene);
        }

        return { true, preparedScene };
    }

    void ForwardRenderer::DrawScene(const PreparedScene& scene)
    {
        auto[createSemaphoreResult, imageAcquiredSemaphore] = context->device->CreateSemaphore();
        HUSKY_ASSERT(createSemaphoreResult == vk::Result::eSuccess);

        auto[acquireResult, index] = context->swapchain->AcquireNextImage(nullptr, imageAcquiredSemaphore.Get());
        HUSKY_ASSERT(acquireResult == vk::Result::eSuccess);

        auto& frameResource = scene.frameResources[index];

        auto &cmdBuffer = frameResource.commandBuffer;

        Array<float32, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        vk::ClearColorValue colorClearValue{ clearColor };
        vk::ClearDepthStencilValue depthStencilClearValue{ maxDepth, 0 };

        Vector<vk::ClearValue> clearValues = { colorClearValue, depthStencilClearValue };

        int32 framebufferWidth = context->swapchain->GetSwapchainCreateInfo().width;
        int32 framebufferHeight = context->swapchain->GetSwapchainCreateInfo().height;

        cmdBuffer
            ->Begin()
            ->BeginInlineRenderPass(scene.renderPass.Get(), frameResource.framebuffer.Get(), clearValues, { { 0, 0 },{ (uint32)framebufferWidth, (uint32)framebufferHeight } });

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

    void ForwardRenderer::PrepareNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh, scene);
        }

        for (const auto& child : node->GetChildren())
        {
            PrepareNode(node, scene);
        }
    }

    void ForwardRenderer::PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, PreparedScene& scene)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            RefPtr<Pipeline> pipeline = primitive->GetPipeline();
            if (pipeline == nullptr)
            {
                pipeline = CreatePipeline(primitive, scene);
                primitive->SetPipeline(pipeline);
            }

            const auto& indexBuffer = primitive->GetIndexBuffer();
            bool indexBufferUploaded = indexBuffer->UploadToDevice(context->device);
            HUSKY_ASSERT(indexBufferUploaded);

            const auto& vertexBuffers = primitive->GetVertexBuffers();
            for (const auto& vertexBuffer : vertexBuffers)
            {
                bool vertexBufferUploaded = vertexBuffer->UploadToDevice(context->device);
                HUSKY_ASSERT(vertexBufferUploaded);
            }

            auto[createUniformBufferResult, createdUniformBuffer] = context->device->CreateBuffer(
                sizeof(MeshUniformBuffer),
                context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
                vk::BufferUsageFlagBits::eUniformBuffer,
                true);

            HUSKY_ASSERT(createUniformBufferResult == vk::Result::eSuccess);

            mesh->SetUniformBuffer(createdUniformBuffer);

            auto [allocateDescriptorSetResult, allocatedDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.meshDescriptorSetLayout);
            HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);

            mesh->SetDescriptorSet(allocatedDescriptorSet);

            DescriptorSetWrites descriptorSetWrites;
            descriptorSetWrites.WriteUniformBufferDescriptors(allocatedDescriptorSet, &scene.meshUniformBufferBinding, { createdUniformBuffer });

            DescriptorSet::Update(descriptorSetWrites);
        }
    }

    void ForwardRenderer::PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& material, PreparedScene& scene)
    {
        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.materialDescriptorSetLayout);
        HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);

        material->SetDescriptorSet(allocatedDescriptorSet);

        DescriptorSetWrites descriptorSetWrites;

        Vector<ImageDescriptorInfo> imageDescriptors;

        // TODO replace null textures with empty texture descriptors

        imageDescriptors.push_back(ImageDescriptorInfo{ material->metallicRoughness.baseColorTexture.texture->GetDeviceImageView(), vk::ImageLayout::eShaderReadOnlyOptimal });
        imageDescriptors.push_back(ImageDescriptorInfo{ material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceImageView(), vk::ImageLayout::eShaderReadOnlyOptimal });
        imageDescriptors.push_back(ImageDescriptorInfo{ material->normalTexture.texture->GetDeviceImageView(), vk::ImageLayout::eShaderReadOnlyOptimal });
        imageDescriptors.push_back(ImageDescriptorInfo{ material->occlusionTexture.texture->GetDeviceImageView(), vk::ImageLayout::eShaderReadOnlyOptimal });
        imageDescriptors.push_back(ImageDescriptorInfo{ material->emissiveTexture.texture->GetDeviceImageView(), vk::ImageLayout::eShaderReadOnlyOptimal });

        descriptorSetWrites.WriteImageDescriptors(allocatedDescriptorSet, &scene.meshUniformBufferBinding, imageDescriptors);

        DescriptorSet::Update(descriptorSetWrites);
    }

    void ForwardRenderer::DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, GlobalDrawContext* drawContext, CommandBuffer* cmdBuffer)
    {
        for (const auto& primitive : mesh->GetPrimitives())
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
            vulkanVertexBuffers.push_back(vertexBuffer->GetDeviceBuffer().Get());
            offsets.push_back(vertexBuffer->GetByteOffset());
        }

        const auto& indexBuffer = primitive->GetIndexBuffer();

        cmdBuffer
            ->BindGraphicsPipeline(pipeline.Get())
            ->BindVertexBuffers(vulkanVertexBuffers, offsets, 0)
            // TODO push material constants
            ->BindIndexBuffer(
                indexBuffer->GetDeviceBuffer().Get(),
                ToVulkanIndexType(indexBuffer->GetIndexType()),
                indexBuffer->GetByteOffset())
            ->DrawIndexed(indexBuffer->GetIndexCount(), 1, 0, 0, 0);
    }

    // Assume that all primitives have the same vertex layout
    RefPtr<Pipeline> ForwardRenderer::CreatePipeline(const RefPtr<SceneV1::Primitive>& primitive, PreparedScene& scene)
    {
        GraphicsPipelineCreateInfo pipelineState;

        auto& vertexShaderStage = pipelineState.shaderStages.emplace_back();
        vertexShaderStage.name = "main";
        vertexShaderStage.shaderModule = scene.vertexShaderModule.Get();
        vertexShaderStage.stage = ShaderStage::Vertex;

        auto& fragmentShaderStage = pipelineState.shaderStages.emplace_back();
        fragmentShaderStage.name = "main";
        fragmentShaderStage.shaderModule = scene.fragmentShaderModule.Get();
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

        pipelineState.renderPass = scene.renderPass.Get();
        pipelineState.layout = scene.pipelineLayout.Get();

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
