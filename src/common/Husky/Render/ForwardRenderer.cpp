//#include <Husky/Render/ForwardRenderer.h>
//#include <Husky/Render/TextureUploader.h>
//
//#include <Husky/PrimitiveTopology.h>
//
//#include <Husky/FileStream.h>
//
//#include <Husky/SceneV1/Scene.h>
//#include <Husky/SceneV1/Node.h>
//#include <Husky/SceneV1/Mesh.h>
//#include <Husky/SceneV1/Primitive.h>
//#include <Husky/SceneV1/Camera.h>
//#include <Husky/SceneV1/AlphaMode.h>
//#include <Husky/SceneV1/PbrMaterial.h>
//#include <Husky/SceneV1/Texture.h>
//#include <Husky/SceneV1/Light.h>
//#include <Husky/SceneV1/Sampler.h>
//#include <Husky/SceneV1/VertexBuffer.h>
//#include <Husky/SceneV1/IndexBuffer.h>
//#include <Husky/SceneV1/AttributeSemantic.h>
//
//#include <Husky/Vulkan/Attachment.h>
//#include <Husky/Vulkan/PhysicalDevice.h>
//#include <Husky/Vulkan/GraphicsDevice.h>
//#include <Husky/Vulkan/DescriptorSetWrites.h>
//#include <Husky/Vulkan/CommandPool.h>
//#include <Husky/Vulkan/DescriptorPool.h>
//#include <Husky/Vulkan/CommandBuffer.h>
//#include <Husky/Vulkan/Swapchain.h>
//#include <Husky/Vulkan/Pipeline.h>
//#include <Husky/Vulkan/DescriptorSetBinding.h>
//#include <Husky/Vulkan/RenderPassCreateInfo.h>
//#include <Husky/Vulkan/SubpassDescription.h>
//#include <Husky/Vulkan/DescriptorSetLayoutCreateInfo.h>
//#include <Husky/Vulkan/PipelineLayoutCreateInfo.h>
//#include <Husky/Vulkan/FramebufferCreateInfo.h>
//#include <Husky/Vulkan/IndexType.h>
//#include <Husky/Vulkan/PipelineCreateInfo.h>
//#include <Husky/Vulkan/Fence.h>
//
//namespace Husky::Render
//{
//    using namespace Vulkan;
//
//    static const Map<SceneV1::AttributeSemantic, int32> SemanticToLocation =
//    {
//        { SceneV1::AttributeSemantic::Position, 0 },
//        { SceneV1::AttributeSemantic::Normal, 1 },
//        { SceneV1::AttributeSemantic::Tangent, 2 },
//        { SceneV1::AttributeSemantic::Texcoord_0, 3 },
//        { SceneV1::AttributeSemantic::Texcoord_1, 4 },
//        { SceneV1::AttributeSemantic::Color_0, 5 },
//    };
//
//    ForwardRenderer::ForwardRenderer(
//        PhysicalDevice* physicalDevice,
//        Surface* surface,
//        int32 aWidth,
//        int32 aHeight)
//        : width(aWidth)
//        , height(aHeight)
//    {
//        context = MakeUnique<ForwardRendererContext>();
//        context->physicalDevice = physicalDevice;
//        context->surface = surface;
//    }
//
//    bool ForwardRenderer::Initialize()
//    {
//        GLSLShaderCompiler::Initialize();
//
//        auto[chooseQueuesResult, queueIndices] = context->physicalDevice->ChooseDeviceQueues(context->surface);
//        if (chooseQueuesResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return false;
//        }
//
//        auto[createDeviceResult, createdDevice] = context->physicalDevice->CreateDevice(std::move(queueIndices), GetRequiredDeviceExtensionNames());
//        if (createDeviceResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return false;
//        }
//
//        context->device = std::move(createdDevice);
//
//        auto& device = context->device;
//        auto indices = context->device->GetQueueIndices();
//
//        auto[createdPresentCommandPoolResult, createdPresentCommandPool] = device->CreateCommandPool(indices->presentQueueFamilyIndex, true, false);
//        if (createdPresentCommandPoolResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return false;
//        }
//
//        context->presentCommandPool = std::move(createdPresentCommandPool);
//
//        auto[swapchainChooseCreateInfoResult, swapchainCreateInfo] = Swapchain::ChooseSwapchainCreateInfo(width, height, context->physicalDevice, context->surface);
//        if (swapchainChooseCreateInfoResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return false;
//        }
//
//        auto[createSwapchainResult, createdSwapchain] = device->CreateSwapchain(swapchainCreateInfo, context->surface);
//        if (createSwapchainResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return false;
//        }
//
//        context->swapchain = std::move(createdSwapchain);
//        return true;
//    }
//
//    bool ForwardRenderer::Deinitialize()
//    {
//        context.release();
//        GLSLShaderCompiler::Deinitialize();
//        return true;
//    }
//    
//    ResultValue<bool, PreparedScene> ForwardRenderer::PrepareScene(const RefPtr<SceneV1::Scene>& scene)
//    {
//        PreparedScene preparedScene;
//
//        preparedScene.scene = scene;
//
//        const auto& sceneProperties = scene->GetSceneProperties();
//
//        const int32 texturesPerMaterial = 5;
//
//        int32 textureDescriptorCount = sceneProperties.materials.size() * texturesPerMaterial;
//        int32 perMeshUBOCount = sceneProperties.meshes.size();
//        int32 perCameraUBOCount = 1;
//        int32 lightsBufferCount = 1;
//
//        // one set per material, one set per mesh, one set for camera, one set for lights
//        int32 maxDescriptorSets = sceneProperties.materials.size() + sceneProperties.meshes.size() + 1 + 1;
//
//        UnorderedMap<vk::DescriptorType, int32> descriptorCount = 
//        {
//            { vk::DescriptorType::eCombinedImageSampler, textureDescriptorCount + 1},
//            { vk::DescriptorType::eUniformBuffer, perMeshUBOCount + perCameraUBOCount + lightsBufferCount + 1},
//        };
//
//        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(maxDescriptorSets, descriptorCount);
//        if (createDescriptorPoolResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.descriptorPool = std::move(createdDescriptorPool);
//
//        auto vertexShaderSource = LoadShaderSource(".\\Shaders\\pbr.vert");
//        auto fragmentShaderSource = LoadShaderSource(".\\Shaders\\pbr.frag");
//
//        auto vertexShaderCompiled = context->shaderCompiler.TryCompileShader(ShaderStage::Vertex, vertexShaderSource, preparedScene.vertexShaderBytecode, {});
//        HUSKY_ASSERT_MSG(vertexShaderCompiled, "Vertex shader failed to compile");
//
//        auto fragmentShaderCompiled = context->shaderCompiler.TryCompileShader(ShaderStage::Fragment, fragmentShaderSource, preparedScene.fragmentShaderBytecode, {});
//        HUSKY_ASSERT_MSG(fragmentShaderCompiled, "Fragment shader failed to compile");
//
//        auto[createVertexShaderModuleResult, createdVertexShaderModule] = context->device->CreateShaderModule(
//            preparedScene.vertexShaderBytecode.data(),
//            preparedScene.vertexShaderBytecode.size() * sizeof(uint32)
//        );
//
//        if (createVertexShaderModuleResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.vertexShaderModule = std::move(createdVertexShaderModule);
//
//        auto[createFragmentShaderModuleResult, createdFragmentShaderModule] = context->device->CreateShaderModule(
//            preparedScene.fragmentShaderBytecode.data(),
//            preparedScene.fragmentShaderBytecode.size() * sizeof(uint32));
//
//        if (createFragmentShaderModuleResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.fragmentShaderModule = std::move(createdFragmentShaderModule);
//
//        Attachment colorAttachment;
//        colorAttachment
//            .SetFormat(context->swapchain->GetFormat())
//            .SetInitialLayout(vk::ImageLayout::eUndefined)
//            .SetLoadOp(vk::AttachmentLoadOp::eClear)
//            .SetStoreOp(vk::AttachmentStoreOp::eStore)
//            .SetSampleCount(SampleCount::e1)
//            .SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
//
//        Attachment depthAttachment;
//        depthAttachment
//            .SetFormat(depthStencilFormat)
//            .SetInitialLayout(vk::ImageLayout::eUndefined)
//            .SetLoadOp(vk::AttachmentLoadOp::eClear)
//            .SetStoreOp(vk::AttachmentStoreOp::eStore)
//            .SetStencilLoadOp(vk::AttachmentLoadOp::eClear)
//            .SetStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
//            .SetSampleCount(SampleCount::e1)
//            .SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
//
//        SubpassDescription subpass;
//        subpass
//            .AddColorAttachment(&colorAttachment, vk::ImageLayout::eColorAttachmentOptimal)
//            .WithDepthStencilAttachment(&depthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);
//
//        RenderPassCreateInfo renderPassCreateInfo;
//        renderPassCreateInfo
//            .AddAttachment(&colorAttachment)
//            .AddAttachment(&depthAttachment)
//            .AddSubpass(std::move(subpass));
//
//        auto[createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
//        if (createRenderPassResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.renderPass = std::move(createdRenderPass);
//
//        preparedScene.cameraUniformBufferBinding
//            .OfType(vk::DescriptorType::eUniformBuffer)
//            .AtStages(ShaderStage::Vertex | ShaderStage::Fragment);
//
//        preparedScene.lightsStorageBufferBinding
//            .OfType(vk::DescriptorType::eUniformBuffer)
//            .AtStages(ShaderStage::Vertex | ShaderStage::Fragment);
//
//        preparedScene.meshUniformBufferBinding
//            .OfType(vk::DescriptorType::eUniformBuffer)
//            .AtStages(ShaderStage::Vertex);
//
//        preparedScene.baseColorTextureBinding
//            .OfType(vk::DescriptorType::eCombinedImageSampler)
//            .AtStages(ShaderStage::Fragment);
//
//        preparedScene.metallicRoughnessTextureBinding
//            .OfType(vk::DescriptorType::eCombinedImageSampler)
//            .AtStages(ShaderStage::Fragment);
//
//        preparedScene.normalTextureBinding
//            .OfType(vk::DescriptorType::eCombinedImageSampler)
//            .AtStages(ShaderStage::Fragment);
//
//        preparedScene.occlusionTextureBinding
//            .OfType(vk::DescriptorType::eCombinedImageSampler)
//            .AtStages(ShaderStage::Fragment);
//
//        preparedScene.emissiveTextureBinding
//            .OfType(vk::DescriptorType::eCombinedImageSampler)
//            .AtStages(ShaderStage::Fragment);
//
//        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
//        cameraDescriptorSetLayoutCreateInfo
//            .AddBinding(&preparedScene.cameraUniformBufferBinding);
//
//        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
//        lightsDescriptorSetLayoutCreateInfo
//            .AddBinding(&preparedScene.lightsStorageBufferBinding);
//
//        DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
//        meshDescriptorSetLayoutCreateInfo
//            .AddBinding(&preparedScene.meshUniformBufferBinding);
//
//        DescriptorSetLayoutCreateInfo materialDescriptorSetLayoutCreateInfo;
//        materialDescriptorSetLayoutCreateInfo
//            .AddBinding(&preparedScene.baseColorTextureBinding)
//            .AddBinding(&preparedScene.metallicRoughnessTextureBinding)
//            .AddBinding(&preparedScene.normalTextureBinding)
//            .AddBinding(&preparedScene.occlusionTextureBinding)
//            .AddBinding(&preparedScene.emissiveTextureBinding);
//
//        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cameraDescriptorSetLayoutCreateInfo);
//        if (createCameraDescriptorSetLayoutResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.cameraDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);
//
//        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
//        if (createLightsDescriptorSetLayoutResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//        
//        preparedScene.lightsDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);
//
//        auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(meshDescriptorSetLayoutCreateInfo);
//        if (createMeshDescriptorSetLayoutResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.meshDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);
//
//        auto[createMaterialDescriptorSetLayoutResult, createdMaterialDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(materialDescriptorSetLayoutCreateInfo);
//        if (createMaterialDescriptorSetLayoutResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.materialDescriptorSetLayout = std::move(createdMaterialDescriptorSetLayout);
//
//
//        // TODO check push constant size limit
//
//        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
//        pipelineLayoutCreateInfo
//            .WithNSetLayouts(4)
//            .AddSetLayout(preparedScene.lightsDescriptorSetLayout)
//            .AddSetLayout(preparedScene.cameraDescriptorSetLayout)
//            .AddSetLayout(preparedScene.meshDescriptorSetLayout)
//            .AddSetLayout(preparedScene.materialDescriptorSetLayout)
//            .WithNPushConstantRanges(1)
//            .AddPushConstantRange(ShaderStage::Fragment, sizeof(MaterialPushConstants));
//
//        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
//        if (createPipelineLayoutResult != vk::Result::eSuccess)
//        {
//            return { false };
//        }
//
//        preparedScene.pipelineLayout = std::move(createdPipelineLayout);
//
//        const auto& swapchainCreateInfo = context->swapchain->GetSwapchainCreateInfo();
//
//        preparedScene.frameResources.reserve(swapchainCreateInfo.imageCount);
//
//        const auto& indices = context->device->GetQueueIndices();
//
//        auto[createdGraphicsCommandPoolResult, createdGraphicsCommandPool] = context->device->CreateCommandPool(indices->graphicsQueueFamilyIndex, false, false);
//        if (createdGraphicsCommandPoolResult != vk::Result::eSuccess)
//        {
//            // TODO
//            return { false };
//        }
//
//        preparedScene.commandPool = std::move(createdGraphicsCommandPool);
//
//        for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
//        {
//            auto& frameResources = preparedScene.frameResources.emplace_back();
//
//            auto[createFrameGraphicsCommandPoolResult, createdFrameGraphicsCommandPool] = context->device->CreateCommandPool(indices->graphicsQueueFamilyIndex, false, false);
//            if (createFrameGraphicsCommandPoolResult != vk::Result::eSuccess)
//            {
//                // TODO
//                return { false };
//            }
//
//            frameResources.graphicsCommandPool = std::move(createdFrameGraphicsCommandPool);
//
//            vk::ImageCreateInfo depthBufferCreateInfo;
//            depthBufferCreateInfo.setFormat(ToVulkanFormat(depthStencilFormat));
//            depthBufferCreateInfo.setArrayLayers(1);
//            depthBufferCreateInfo.setImageType(vk::ImageType::e2D);
//            depthBufferCreateInfo.setExtent({ (uint32)swapchainCreateInfo.width, (uint32)swapchainCreateInfo.height, 1 });
//            depthBufferCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
//            depthBufferCreateInfo.setMipLevels(1);
//            depthBufferCreateInfo.setQueueFamilyIndexCount(1);
//            depthBufferCreateInfo.setPQueueFamilyIndices(&indices->graphicsQueueFamilyIndex);
//            depthBufferCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
//            depthBufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
//            depthBufferCreateInfo.setTiling(vk::ImageTiling::eOptimal);
//            depthBufferCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);
//
//            auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = context->device->CreateImage(depthBufferCreateInfo);
//            if (createDepthStencilBufferResult != vk::Result::eSuccess)
//            {
//                return { false };
//            }
//
//            frameResources.depthStencilBuffer = std::move(createdDepthStencilBuffer);
//
//            auto[createDepthStencilBufferViewResult, createdDepthStencilBufferView] = context->device->CreateImageView(frameResources.depthStencilBuffer);
//            if (createDepthStencilBufferViewResult != vk::Result::eSuccess)
//            {
//                return { false };
//            }
//
//            frameResources.depthStencilBufferView = std::move(createdDepthStencilBufferView);
//
//            FramebufferCreateInfo framebufferCreateInfo(preparedScene.renderPass, swapchainCreateInfo.width, swapchainCreateInfo.height, 1);
//            framebufferCreateInfo
//                .AddAttachment(&colorAttachment, context->swapchain->GetImageView(i))
//                .AddAttachment(&depthAttachment, frameResources.depthStencilBufferView);
//
//            auto[createFramebufferResult, createdFramebuffer] = context->device->CreateFramebuffer(framebufferCreateInfo);
//            if (createFramebufferResult != vk::Result::eSuccess)
//            {
//                return { false };
//            }
//
//            frameResources.framebuffer = std::move(createdFramebuffer);
//
//            auto[createFenceResult, createdFence] = context->device->CreateFence();
//            if (createFenceResult != vk::Result::eSuccess)
//            {
//                return { false };
//            }
//
//            auto[allocateResult, allocatedBuffer] = frameResources.graphicsCommandPool->AllocateCommandBuffer(CommandBufferLevel::Primary);
//            if (allocateResult != vk::Result::eSuccess)
//            {
//                return { false };
//            }
//
//            frameResources.commandBuffer = std::move(allocatedBuffer);
//
//            frameResources.fence = std::move(createdFence);
//
//            auto[createSemaphoreResult, createdSemaphore] = context->device->CreateSemaphore();
//            if (createSemaphoreResult != vk::Result::eSuccess)
//            {
//                return { false };
//            }
//
//            frameResources.semaphore = std::move(createdSemaphore);
//        }
//
//        const auto& textures = sceneProperties.textures;
//
//        Vector<SceneV1::Texture*> texturesVector;
//        for (const auto& texture : textures)
//        {
//            texturesVector.push_back(texture);
//        }
//
//        TextureUploader textureUploader{ context->device, preparedScene.commandPool };
//        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);
//
//        HUSKY_ASSERT(uploadTexturesSucceeded);
//
//        // TODO semaphores
//
//        Submission uploadTexturesSubmission;
//        for (const auto& buffer : uploadTexturesResult.commandBuffers)
//        {
//            uploadTexturesSubmission.commandBuffers.push_back(buffer);
//        }
//
//        context->device->GetGraphicsQueue()->Submit(uploadTexturesSubmission);
//        context->device->GetGraphicsQueue()->WaitIdle();
//
//        const auto& nodes = scene->GetNodes();
//
//        for (const auto& node : nodes)
//        {
//            if (node->GetCamera() != nullptr)
//            {
//                HUSKY_ASSERT_MSG(preparedScene.cameraNode == nullptr, "Only one camera node is allowed");
//                preparedScene.cameraNode = node;
//                PrepareCameraNode(node, preparedScene);
//            }
//            else if(node->GetMesh() != nullptr)
//            {
//                PrepareMeshNode(node, preparedScene);
//            }
//            else if (node->GetLight() != nullptr)
//            {
//                PrepareLightNode(node, preparedScene);
//            }
//            else
//            {
//                //HUSKY_ASSERT(false);
//            }
//        }
//
//        for (const auto& material : sceneProperties.materials)
//        {
//            PrepareMaterial(material, preparedScene);
//        }
//
//        PrepareLights(preparedScene);
//
//        return { true, preparedScene };
//    }
//
//    void ForwardRenderer::UpdateScene(PreparedScene& scene)
//    {
//        Mat4x4 identity = glm::mat4(1.0f);
//        for (const auto& node : scene.scene->GetNodes())
//        {
//            UpdateNode(node, identity, scene);
//        }
//    }
//
//    void ForwardRenderer::DrawScene(const PreparedScene& scene)
//    {
//        auto[createSemaphoreResult, imageAcquiredSemaphore] = context->device->CreateSemaphore();
//        HUSKY_ASSERT(createSemaphoreResult == vk::Result::eSuccess);
//
//        auto[acquireResult, index] = context->swapchain->AcquireNextImage(nullptr, imageAcquiredSemaphore);
//        HUSKY_ASSERT(acquireResult == vk::Result::eSuccess);
//
//        auto& frameResource = scene.frameResources[index];
//
//        auto &cmdBuffer = frameResource.commandBuffer;
//
//        Array<float32, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
//        vk::ClearColorValue colorClearValue{ clearColor };
//        vk::ClearDepthStencilValue depthStencilClearValue{ maxDepth, 0 };
//
//        Vector<vk::ClearValue> clearValues = { colorClearValue, depthStencilClearValue };
//
//        int32 framebufferWidth = context->swapchain->GetSwapchainCreateInfo().width;
//        int32 framebufferHeight = context->swapchain->GetSwapchainCreateInfo().height;
//
//        cmdBuffer
//            ->Begin()
//            ->SetViewport({ 0, 0, (float32)framebufferWidth, (float32)framebufferHeight, 0.0f, 1.0f })
//            ->SetScissor({ {0, 0}, {(uint32)framebufferWidth, (uint32)framebufferHeight} })
//            ->BindDescriptorSet(scene.pipelineLayout, 0, scene.lightsDescriptorSet)
//            ->BindDescriptorSet(scene.pipelineLayout, 1, scene.cameraNode->GetCamera()->GetDescriptorSet())
//            ->BeginInlineRenderPass(
//                scene.renderPass,
//                frameResource.framebuffer,
//                clearValues,
//                { { 0, 0 },{ (uint32)framebufferWidth, (uint32)framebufferHeight } });
//
//        for (const auto& node : scene.scene->GetNodes())
//        {
//            DrawNode(node, scene, cmdBuffer);
//        }
//
//        cmdBuffer
//            ->EndRenderPass()
//            ->End();
//
//        Submission submission;
//        submission.commandBuffers = { cmdBuffer };
//        submission.fence = frameResource.fence;
//        submission.waitOperations = { { imageAcquiredSemaphore, vk::PipelineStageFlagBits::eColorAttachmentOutput } };
//        submission.signalSemaphores = { frameResource.semaphore };
//
//        context->device->GetGraphicsQueue()->Submit(submission);
//
//        frameResource.fence->Wait();
//        frameResource.fence->Reset();
//
//        frameResource.graphicsCommandPool->Reset();
//
//        PresentSubmission presentSubmission;
//        presentSubmission.index = index;
//        presentSubmission.swapchain = context->swapchain;
//        presentSubmission.waitSemaphores = { frameResource.semaphore };
//
//        context->device->GetPresentQueue()->Present(presentSubmission);
//    }
//
//    void Husky::Render::ForwardRenderer::PrepareCameraNode(const RefPtr<SceneV1::Node>& node, PreparedScene & scene)
//    {
//        const auto& camera = node->GetCamera();
//
//        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(
//            sizeof(CameraUniformBuffer),
//            context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
//            vk::BufferUsageFlagBits::eUniformBuffer,
//            true);
//        HUSKY_ASSERT(createBufferResult == vk::Result::eSuccess);
//
//        auto[mapMemoryResult, mappedMemory] = createdBuffer->MapMemory(sizeof(CameraUniformBuffer), 0);
//        HUSKY_ASSERT(mapMemoryResult == vk::Result::eSuccess);
//
//        auto[createDescriptorSetResult, createdDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.cameraDescriptorSetLayout);
//        HUSKY_ASSERT(createDescriptorSetResult == vk::Result::eSuccess);
//
//        camera->SetUniformBuffer(createdBuffer);
//        camera->SetDescriptorSet(createdDescriptorSet);
//
//        DescriptorSetWrites descriptorSetWrites;
//        descriptorSetWrites.WriteUniformBufferDescriptors(createdDescriptorSet, &scene.cameraUniformBufferBinding, { createdBuffer });
//        DescriptorSet::Update(descriptorSetWrites);
//    }
//
//    void ForwardRenderer::PrepareMeshNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene)
//    {
//        const auto& mesh = node->GetMesh();
//
//        if (mesh != nullptr)
//        {
//            PrepareMesh(mesh, scene);
//        }
//
//        // TODO sort out node hierarchy
//        for (const auto& child : node->GetChildren())
//        {
//            PrepareMeshNode(node, scene);
//        }
//    }
//
//    void Husky::Render::ForwardRenderer::PrepareLightNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene)
//    {
//        const auto& light = node->GetLight();
//
//        if (light!= nullptr)
//        {
//            PrepareLight(light, scene);
//        }
//
//        HUSKY_ASSERT_MSG(node->GetChildren().empty(), "Don't add children to light nodes");
//    }
//
//    void Husky::Render::ForwardRenderer::PrepareNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene)
//    {
//    }
//
//    void ForwardRenderer::PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, PreparedScene& scene)
//    {
//        for (const auto& primitive : mesh->GetPrimitives())
//        {
//            RefPtr<Pipeline> pipeline = primitive->GetPipeline();
//            if (pipeline == nullptr)
//            {
//                pipeline = CreatePipeline(primitive, scene);
//                primitive->SetPipeline(pipeline);
//            }
//
//            auto[createUniformBufferResult, createdUniformBuffer] = context->device->CreateBuffer(
//                sizeof(MeshUniformBuffer),
//                context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
//                vk::BufferUsageFlagBits::eUniformBuffer,
//                true);
//
//            HUSKY_ASSERT(createUniformBufferResult == vk::Result::eSuccess);
//
//            auto[mapMemoryResult, mappedMemory] = createdUniformBuffer->MapMemory(sizeof(MeshUniformBuffer), 0);
//            HUSKY_ASSERT(mapMemoryResult == vk::Result::eSuccess);
//
//            auto [allocateDescriptorSetResult, allocatedDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.meshDescriptorSetLayout);
//            HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);
//
//            mesh->SetUniformBuffer(createdUniformBuffer);
//            mesh->SetDescriptorSet(allocatedDescriptorSet);
//
//            DescriptorSetWrites descriptorSetWrites;
//            descriptorSetWrites.WriteUniformBufferDescriptors(allocatedDescriptorSet, &scene.meshUniformBufferBinding, { createdUniformBuffer });
//
//            DescriptorSet::Update(descriptorSetWrites);
//        }
//    }
//
//    void Husky::Render::ForwardRenderer::PrepareLight(const RefPtr<SceneV1::Light>& light, PreparedScene& scene)
//    {
//        scene.lights.push_back(light);
//    }
//
//    void ForwardRenderer::PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& material, PreparedScene& scene)
//    {
//        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.materialDescriptorSetLayout);
//        HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);
//
//        material->SetDescriptorSet(allocatedDescriptorSet);
//
//        DescriptorSetWrites descriptorSetWrites;
//
//        Vector<ImageDescriptorInfo> imageDescriptors;
//
//        if (material->HasBaseColorTexture())
//        {
//            descriptorSetWrites.WriteCombinedImageDescriptors(
//                material->GetDescriptorSet(),
//                &scene.baseColorTextureBinding,
//                { ToVulkanImageDescriptorInfo(material->metallicRoughness.baseColorTexture) });
//        }
//
//        if (material->HasMetallicRoughnessTexture())
//        {
//            descriptorSetWrites.WriteCombinedImageDescriptors(
//                material->GetDescriptorSet(),
//                &scene.metallicRoughnessTextureBinding,
//                { ToVulkanImageDescriptorInfo(material->metallicRoughness.metallicRoughnessTexture) });
//        }
//
//        if (material->HasNormalTexture())
//        {
//            descriptorSetWrites.WriteCombinedImageDescriptors(
//                material->GetDescriptorSet(),
//                &scene.normalTextureBinding,
//                { ToVulkanImageDescriptorInfo(material->normalTexture) });
//        }
//        
//        if (material->HasOcclusionTexture())
//        {
//            descriptorSetWrites.WriteCombinedImageDescriptors(
//                material->GetDescriptorSet(),
//                &scene.occlusionTextureBinding,
//                { ToVulkanImageDescriptorInfo(material->occlusionTexture) });
//        }
//        
//        if (material->HasEmissiveTexture())
//        {
//            descriptorSetWrites.WriteCombinedImageDescriptors(
//                material->GetDescriptorSet(),
//                &scene.emissiveTextureBinding,
//                { ToVulkanImageDescriptorInfo(material->emissiveTexture) });
//        }
//
//        DescriptorSet::Update(descriptorSetWrites);
//    }
//
//    void Husky::Render::ForwardRenderer::PrepareLights(PreparedScene& scene)
//    {
//        int32 lightsCount = scene.lights.size();
//        int32 lightsBufferSize = sizeof(LightUniform) * lightsCount;
//
//        auto[createLightsBufferResult, createdLightsBuffer] = context->device->CreateBuffer(
//            lightsBufferSize,
//            context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
//            vk::BufferUsageFlagBits::eUniformBuffer, true);
//
//        auto[mapMemoryResult, mappedMemory] = createdLightsBuffer->MapMemory(lightsBufferSize, 0);
//        HUSKY_ASSERT(mapMemoryResult == vk::Result::eSuccess);
//
//        memset(mappedMemory, 0, lightsBufferSize);
//
//        HUSKY_ASSERT(createLightsBufferResult == vk::Result::eSuccess);
//        scene.lightsBuffer = createdLightsBuffer;
//
//        auto[createDescriptorSetResult, createdDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.lightsDescriptorSetLayout);
//        HUSKY_ASSERT(createDescriptorSetResult == vk::Result::eSuccess);
//        scene.lightsDescriptorSet = createdDescriptorSet;
//
//        DescriptorSetWrites descriptorSetWrites;
//
//        descriptorSetWrites.WriteUniformBufferDescriptors(createdDescriptorSet, &scene.lightsStorageBufferBinding, { createdLightsBuffer });
//
//        DescriptorSet::Update(descriptorSetWrites);
//    }
//
//    void Husky::Render::ForwardRenderer::UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform, PreparedScene& scene)
//    {
//        const auto& mesh = node->GetMesh();
//
//        Mat4x4 localTransformMatrix;
//        const auto& localTransform = node->GetTransform();
////        if (std::holds_alternative<Mat4x4>(localTransform))
////        {
////            localTransformMatrix = std::get<Mat4x4>(localTransform);
////        }
////        else
////        {
////            const auto& transformProperties = std::get<SceneV1::TransformProperties>(localTransform);
////
////            localTransformMatrix
////                = glm::translate(transformProperties.translation)
////                * glm::toMat4(transformProperties.rotation)
////                * glm::scale(transformProperties.scale);
////        }
//
//        if (localTransform.hasMatrix)
//        {
//            localTransformMatrix = localTransform.matrix;
//        }
//        else
//        {
//            const auto& transformProperties = localTransform.properties;
//
//            localTransformMatrix
//                = glm::translate(transformProperties.translation)
//                * glm::toMat4(transformProperties.rotation)
//                * glm::scale(transformProperties.scale);
//        }
//
//        Mat4x4 transform = parentTransform * localTransformMatrix;
//
//        if (mesh != nullptr)
//        {
//            UpdateMesh(mesh, transform, scene);
//        }
//
//        const auto& camera = node->GetCamera();
//
//        if (camera != nullptr)
//        {
//            UpdateCamera(camera, transform, scene);
//        }
//
//        const auto& light = node->GetLight();
//        if (light != nullptr)
//        {
//            UpdateLight(light, transform, scene);
//        }
//
//        for (const auto& child : node->GetChildren())
//        {
//            UpdateNode(child, transform, scene);
//        }
//    }
//
//    void Husky::Render::ForwardRenderer::UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform, PreparedScene & scene)
//    {
//        const auto& buffer = mesh->GetUniformBuffer();
//
//        MeshUniformBuffer meshUniformBuffer;
//        //meshUniformBuffer.transform = glm::transpose(transform);
//        meshUniformBuffer.transform = (transform);
//
//        // TODO template function member in buffer to write updates
//        memcpy(buffer->GetMappedMemory(), &meshUniformBuffer, sizeof(MeshUniformBuffer));
//    }
//
//    void Husky::Render::ForwardRenderer::UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform, PreparedScene & scene)
//    {
//        const auto& buffer = camera->GetUniformBuffer();
//
//        camera->SetCameraViewMatrix(transform);
//
//        CameraUniformBuffer cameraUniformBuffer;
//        cameraUniformBuffer.view = camera->GetCameraViewMatrix();
//        cameraUniformBuffer.projection = camera->GetCameraProjectionMatrix();
//        cameraUniformBuffer.position = Vec4{ camera->GetCameraPosition(), 1.0 };
//        // TODO
//        memcpy(buffer->GetMappedMemory(), &cameraUniformBuffer, sizeof(CameraUniformBuffer));
//    }
//
//    void Husky::Render::ForwardRenderer::UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform, PreparedScene& scene)
//    {
//        auto viewTransform = scene.cameraNode->GetCamera()->GetCameraViewMatrix();
//
//        LightUniform lightUniform;
//
//        lightUniform.positionWS = transform * Vec4{ 0.0, 0.0, 0.0, 1.0 };
//        lightUniform.directionWS = transform * Vec4{ light->GetDirection().value_or(Vec3{0, 0, 1}), 0.0 };
//        lightUniform.positionVS = viewTransform * lightUniform.positionWS;
//        lightUniform.directionVS = viewTransform * lightUniform.directionWS;
//        lightUniform.color = Vec4{ light->GetColor().value_or(Vec3{1.0, 1.0, 1.0}), 1.0 };
//        lightUniform.enabled = light->IsEnabled() ? 1 : 0;
//        lightUniform.type = static_cast<int32>(light->GetType());
//        lightUniform.spotlightAngle = light->GetSpotlightAngle().value_or(0.0);
//        lightUniform.range = light->GetRange().value_or(0.0);
//        lightUniform.intensity = light->GetIntensity();
//        lightUniform.enabled = true ;
//
//        int32 offset = sizeof(LightUniform)*light->GetIndex();
//        memcpy((Byte*)scene.lightsBuffer->GetMappedMemory() + offset, &lightUniform, sizeof(LightUniform));
//    }
//
//    void Husky::Render::ForwardRenderer::DrawNode(const RefPtr<SceneV1::Node>& node, const PreparedScene& scene, CommandBuffer* cmdBuffer)
//    {
//        const auto& mesh = node->GetMesh();
//        if (mesh != nullptr)
//        {
//            DrawMesh(mesh, scene, cmdBuffer);
//        }
//
//        for (const auto& child : node->GetChildren())
//        {
//            DrawNode(child, scene, cmdBuffer);
//        }
//    }
//
//    void ForwardRenderer::DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, const PreparedScene& scene, CommandBuffer* cmdBuffer)
//    {
//        cmdBuffer->BindDescriptorSet(scene.pipelineLayout, 2, mesh->GetDescriptorSet());
//
//        for (const auto& primitive : mesh->GetPrimitives())
//        {
//            if (primitive->GetMaterial()->alphaMode != SceneV1::AlphaMode::Blend)
//            {
//                DrawPrimitive(primitive, scene, cmdBuffer);
//            }
//        }
//
//        // TODO sort by distance to camera
//        for (const auto& primitive : mesh->GetPrimitives())
//        {
//            if (primitive->GetMaterial()->alphaMode == SceneV1::AlphaMode::Blend)
//            {
//                DrawPrimitive(primitive, scene, cmdBuffer);
//            }
//        }
//    }
//
//    void ForwardRenderer::DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, const PreparedScene& scene, CommandBuffer* cmdBuffer)
//    {
//        RefPtr<Pipeline> pipeline = primitive->GetPipeline();
//
//        const auto& vertexBuffers = primitive->GetVertexBuffers();
//
//        Vector<DeviceBuffer*> vulkanVertexBuffers;
//        Vector<int64> offsets;
//        vulkanVertexBuffers.reserve(vertexBuffers.size());
//        offsets.reserve(vertexBuffers.size());
//
//        for (const auto& vertexBuffer : vertexBuffers)
//        {
//            vulkanVertexBuffers.push_back(vertexBuffer.backingBuffer->GetDeviceBuffer());
//            offsets.push_back(vertexBuffer.byteOffset);
//        }
//
//        HUSKY_ASSERT(primitive->GetIndexBuffer().has_value());
//        const auto& indexBuffer = *primitive->GetIndexBuffer();
//
//        const auto& material = primitive->GetMaterial();
//
//        MaterialPushConstants materialPushConstants = GetMaterialPushConstants(material);
//
//        cmdBuffer
//            ->BindGraphicsPipeline(pipeline)
//            ->BindVertexBuffers(vulkanVertexBuffers, offsets, 0)
//            ->BindDescriptorSet(scene.pipelineLayout, 3, primitive->GetMaterial()->GetDescriptorSet())
//            ->BindIndexBuffer(
//                indexBuffer.backingBuffer->GetDeviceBuffer(),
//                ToVulkanIndexType(indexBuffer.indexType),
//                0)
//            ->PushConstants(scene.pipelineLayout, ShaderStage::Fragment, 0, materialPushConstants)
//            ->DrawIndexed(indexBuffer.count, 1, 0, 0, 0);
//    }
//
//    ImageDescriptorInfo ForwardRenderer::ToVulkanImageDescriptorInfo(const SceneV1::TextureInfo& textureInfo)
//    {
//        return ImageDescriptorInfo{ textureInfo.texture->GetDeviceImageView(), textureInfo.texture->GetDeviceSampler(), vk::ImageLayout::eShaderReadOnlyOptimal };
//    }
//
//    MaterialPushConstants ForwardRenderer::GetMaterialPushConstants(SceneV1::PbrMaterial * material)
//    {
//        MaterialPushConstants materialPushConstants;
//        materialPushConstants.baseColorFactor = material->metallicRoughness.baseColorFactor;
//        materialPushConstants.metallicFactor = material->metallicRoughness.metallicFactor;
//        materialPushConstants.roughnessFactor = material->metallicRoughness.roughnessFactor;
//        materialPushConstants.normalScale = material->normalTexture.scale;
//        materialPushConstants.occlusionStrength = material->occlusionTexture.strength;
//        materialPushConstants.emissiveFactor = material->emissiveFactor;
//        materialPushConstants.alphaCutoff = material->alphaCutoff;
//        return materialPushConstants;
//    }
//
//    Vector<Byte> Husky::Render::ForwardRenderer::LoadShaderSource(const FilePath& path)
//    {
//        FileStream fileStream{ path, FileOpenModes::Read };
//        auto fileSize = fileStream.GetSize();
//        Vector<Byte> result;
//        result.resize(fileSize + 1); // +1 for null termination
//        auto bytesRead = fileStream.Read(result.data(), fileSize, sizeof(Byte));
//        HUSKY_ASSERT(bytesRead == fileSize);
//        return result;
//    }
//
//    // Assume that all primitives have the same vertex layout
//    RefPtr<Pipeline> ForwardRenderer::CreatePipeline(const RefPtr<SceneV1::Primitive>& primitive, PreparedScene& scene)
//    {
//        GraphicsPipelineCreateInfo pipelineState;
//
//        auto& vertexShaderStage = pipelineState.shaderStages.emplace_back();
//        vertexShaderStage.name = "main";
//        vertexShaderStage.shaderModule = scene.vertexShaderModule;
//        vertexShaderStage.stage = ShaderStage::Vertex;
//
//        auto& fragmentShaderStage = pipelineState.shaderStages.emplace_back();
//        fragmentShaderStage.name = "main";
//        fragmentShaderStage.shaderModule = scene.fragmentShaderModule;
//        fragmentShaderStage.stage = ShaderStage::Fragment;
//
//        const auto& vertexBuffers = primitive->GetVertexBuffers();
//        pipelineState.vertexInputState.bindingDescriptions.resize(vertexBuffers.size());
//        for (int i = 0; i < vertexBuffers.size(); i++)
//        {
//            const auto& vertexBuffer = vertexBuffers[i];
//            auto& bindingDescription = pipelineState.vertexInputState.bindingDescriptions[i];
//            bindingDescription.binding = i;
//            bindingDescription.stride = vertexBuffer.stride;
//            bindingDescription.inputRate = vk::VertexInputRate::eVertex;
//        }
//
//        const auto& attributes = primitive->GetAttributes();
//        pipelineState.vertexInputState.attributeDescriptions.reserve(attributes.size());
//        for (const auto& attribute : attributes)
//        {
//            auto& attributeDescription = pipelineState.vertexInputState.attributeDescriptions.emplace_back();
//            attributeDescription.location = SemanticToLocation.at(attribute.semantic);
//            attributeDescription.binding = attribute.vertexBufferIndex;
//            attributeDescription.format = ToVulkanFormat(attribute.format);
//            attributeDescription.offset = attribute.offset;
//        }
//
//        // TODO
//        pipelineState.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
//
//        const auto& material = primitive->GetMaterial();
//
//        if (material->doubleSided)
//        {
//            pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
//        }
//        else
//        {
//            pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
//        }
//
//        // TEST CODE
//        pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
//
//        pipelineState.depthStencilState.depthTestEnable = true;
//        pipelineState.depthStencilState.depthWriteEnable = true;
//        pipelineState.depthStencilState.depthCompareOp = vk::CompareOp::eLess;
//
//        auto &attachmentBlendState = pipelineState.colorBlendState.attachments.emplace_back();
//        attachmentBlendState.blendEnable = material->alphaMode == SceneV1::AlphaMode::Blend;
//        if (attachmentBlendState.blendEnable)
//        {
//            attachmentBlendState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
//            attachmentBlendState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
//            attachmentBlendState.colorBlendOp = vk::BlendOp::eAdd;
//            attachmentBlendState.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
//            attachmentBlendState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
//            attachmentBlendState.alphaBlendOp = vk::BlendOp::eAdd;
//        }
//
//        pipelineState.renderPass = scene.renderPass;
//        pipelineState.layout = scene.pipelineLayout;
//
//        pipelineState.viewportState.viewports.emplace_back();
//        pipelineState.viewportState.scissors.emplace_back();
//
//        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eScissor);
//        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eViewport);
//
//        auto[createPipelineResult, createdPipeline] = context->device->CreateGraphicsPipeline(pipelineState);
//        if (createPipelineResult != vk::Result::eSuccess)
//        {
//            HUSKY_ASSERT(false);
//        }
//
//        return createdPipeline;
//    }
//
//    Vector<const char8*> ForwardRenderer::GetRequiredDeviceExtensionNames() const
//    {
//        Vector<const char8*> requiredExtensionNames;
//
//        requiredExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
//
//        return requiredExtensionNames;
//    }
//}
