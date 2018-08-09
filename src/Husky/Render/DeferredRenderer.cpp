#include <Husky/Render/DeferredRenderer.h>
#include <Husky/Render/TextureUploader.h>

#include <Husky/PrimitiveTopology.h>

#include <Husky/FileStream.h>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/AlphaMode.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Light.h>
#include <Husky/SceneV1/Sampler.h>
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

    // Fullscreen quad for triangle list
    static const Vector<QuadVertex> fullscreenQuadVertices =
    {
        {{-1.0f, +1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom left
        {{+1.0f, +1.0f, 0.0f}, {1.0f, 1.0f}}, // bottom right
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // top left

        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},  // top left
        {{+1.0f, +1.0f, 0.0f}, {1.0f, 1.0f}},  // bottom right
        {{+1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // top right
    };

    // Think about passing these numbers through shader defines
    static const Map<SceneV1::AttributeSemantic, int32> SemanticToLocation =
    {
        { SceneV1::AttributeSemantic::Position, 0 },
        { SceneV1::AttributeSemantic::Normal, 1 },
        { SceneV1::AttributeSemantic::Tangent, 2 },
        { SceneV1::AttributeSemantic::Texcoord_0, 3 },
        { SceneV1::AttributeSemantic::Texcoord_1, 4 },
        { SceneV1::AttributeSemantic::Color_0, 5 },
    };

    static const Map<SceneV1::AttributeSemantic, ShaderDefine> SemanticToFlag =
    {
        { SceneV1::AttributeSemantic::Position, ShaderDefine::Empty },
        { SceneV1::AttributeSemantic::Normal, ShaderDefine::HasNormal},
        { SceneV1::AttributeSemantic::Tangent, ShaderDefine::HasTangent },
        { SceneV1::AttributeSemantic::Texcoord_0, ShaderDefine::HasTexCoord0 },
        { SceneV1::AttributeSemantic::Texcoord_1, ShaderDefine::HasTexCoord1 },
        { SceneV1::AttributeSemantic::Color_0, ShaderDefine::HasColor},
    };

    static UnorderedMap<ShaderDefine, String> FlagToString =
    {
        //{ ShaderDefine::HasPosition, "HAS_POSITION" },
        { ShaderDefine::HasNormal, "HAS_NORMAL" },
        { ShaderDefine::HasTangent, "HAS_TANGENT" },
        { ShaderDefine::HasTexCoord0, "HAS_TEXCOORD_0" },
        { ShaderDefine::HasTexCoord1, "HAS_TEXCOORD_0" },
        { ShaderDefine::HasColor, "HAS_COLOR" },
        { ShaderDefine::HasBitangentDirection , "HAS_BITANGENT_DIRECTION"},
        { ShaderDefine::HasBaseColorTexture, "HAS_BASE_COLOR_TEXTURE" },
        { ShaderDefine::HasMetallicRoughnessTexture, "HAS_METALLIC_ROUGHNESS_TEXTURE" },
        { ShaderDefine::HasNormalTexture, "HAS_NORMAL_TEXTURE" },
        { ShaderDefine::HasOcclusionTexture, "HAS_OCCLUSION_TEXTURE" },
        { ShaderDefine::HasEmissiveTexture, "HAS_EMISSIVE_TEXTURE" },
        { ShaderDefine::AlphaMask, "ALPHA_MASK" },
    };

    void ShaderDefines::AddFlag(ShaderDefine flag)
    {
        AddDefine(flag, "1");
    }

    void ShaderDefines::AddDefine(ShaderDefine define, const String& value)
    {
        if (define != ShaderDefine::Empty)
        {
            defines[FlagToString.at(define)] = value;
        }
    }

    DeferredRenderer::DeferredRenderer(
        PhysicalDevice* physicalDevice,
        const RefPtr<Surface>& surface,
        int32 aWidth,
        int32 aHeight)
        : width(aWidth)
        , height(aHeight)
    {
        context = MakeUnique<DeferredRendererContext>();
        context->physicalDevice = physicalDevice;
        context->surface = surface;
    }

    bool DeferredRenderer::Initialize()
    {
        GLSLShaderCompiler::Initialize();

        auto[chooseQueuesResult, queueIndices] = context->physicalDevice->ChooseDeviceQueues(context->surface);
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

        auto[swapchainChooseCreateInfoResult, swapchainCreateInfo] = Swapchain::ChooseSwapchainCreateInfo(width, height, context->physicalDevice, context->surface);
        if (swapchainChooseCreateInfoResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        auto[createSwapchainResult, createdSwapchain] = device->CreateSwapchain(swapchainCreateInfo, context->surface);
        if (createSwapchainResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        context->swapchain = std::move(createdSwapchain);

        Vector<Format> depthFormats =
        {
            Format::D32SfloatS8Uint,
            Format::D24UnormS8Uint,
            Format::D16UnormS8Uint,
        };

        auto supportedDepthFormats = context->physicalDevice->GetSupportedDepthStencilFormats(depthFormats);
        HUSKY_ASSERT_MSG(!depthFormats.empty(), "No supported depth formats");
        depthStencilFormat = depthFormats.front();

        auto[createSemaphoreResult, imageAcquiredSemaphore] = context->device->CreateSemaphore();
        if(createSemaphoreResult != vk::Result::eSuccess)
        {
            return false;
        }

        context->presentSemaphore = std::move(imageAcquiredSemaphore);

        return true;
    }

    bool DeferredRenderer::Deinitialize()
    {
        context.release();
        GLSLShaderCompiler::Deinitialize();
        return true;
    }
    
    ResultValue<bool, DeferredPreparedScene> DeferredRenderer::PrepareScene(const RefPtr<SceneV1::Scene>& scene)
    {
        DeferredPreparedScene preparedScene;

        preparedScene.scene = scene;

        const auto& sceneProperties = scene->GetSceneProperties();

        int32 perCameraUBOCount = 1;
        int32 maxDescriptorSets = 1;

        UnorderedMap<vk::DescriptorType, int32> descriptorCount =
        {
            { vk::DescriptorType::eUniformBuffer, perCameraUBOCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(maxDescriptorSets, descriptorCount);
        if (createDescriptorPoolResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.descriptorPool = std::move(createdDescriptorPool);

        preparedScene.cameraUniformBufferBinding
            .OfType(vk::DescriptorType::eUniformBuffer)
            .AtStages(ShaderStage::Vertex | ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo cameraDescriptorSetLayoutCreateInfo;
        cameraDescriptorSetLayoutCreateInfo
            .AddBinding(&preparedScene.cameraUniformBufferBinding);

        auto[createCameraDescriptorSetLayoutResult, createdCameraDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(cameraDescriptorSetLayoutCreateInfo);
        if (createCameraDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        preparedScene.cameraDescriptorSetLayout = std::move(createdCameraDescriptorSetLayout);

        auto [gBufferResourcesPrepared, preparedGBufferResources] = PrepareGBufferPassResources(preparedScene);
        if (gBufferResourcesPrepared)
        {
            preparedScene.gBuffer = std::move(preparedGBufferResources);
        }

        auto[lightingResourcesPrepared, preparedLightingResources] = PrepareLightingPassResources(preparedScene);
        if (lightingResourcesPrepared)
        {
            preparedScene.lighting = std::move(preparedLightingResources);
        }

        // compatible with gbuffer and lighting pipeline layouts
        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .WithNSetLayouts(1)
            .AddSetLayout(preparedScene.cameraDescriptorSetLayout);

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

        uint32 swapchainWidth = swapchainCreateInfo.width;
        uint32 swapchainHeight = swapchainCreateInfo.height;

        for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
        {
            auto& frameResources = preparedScene.frameResources.emplace_back();

            auto [allocateGBufferDescriptorSetResult, allocatedGBufferDescriptorset] = 
                preparedScene.lighting.descriptorPool->AllocateDescriptorSet(preparedScene.lighting.gbufferDescriptorSetLayout);
            if (allocateGBufferDescriptorSetResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.gbufferDescriptorSet = std::move(allocatedGBufferDescriptorset);

            auto[createFrameGraphicsCommandPoolResult, createdFrameGraphicsCommandPool] = context->device->CreateCommandPool(indices->graphicsQueueFamilyIndex, false, false);
            if (createFrameGraphicsCommandPoolResult != vk::Result::eSuccess)
            {
                // TODO
                return { false };
            }

            frameResources.graphicsCommandPool = std::move(createdFrameGraphicsCommandPool);

            auto [offscreenImagesCreated, createdOffscreenImages] = CreateOffscreenImages();
            if (!offscreenImagesCreated)
            {
                return { false };
            }

            frameResources.offscreen = std::move(createdOffscreenImages);

            DescriptorSetWrites gbufferDescriptorSetWrites;

            gbufferDescriptorSetWrites.WriteCombinedImageDescriptors(
                frameResources.gbufferDescriptorSet,
                &preparedScene.lighting.baseColorImageBinding,
                { { frameResources.offscreen.baseColorImageView, preparedScene.lighting.baseColorSampler, vk::ImageLayout::eShaderReadOnlyOptimal } });

            gbufferDescriptorSetWrites.WriteCombinedImageDescriptors(
                frameResources.gbufferDescriptorSet,
                &preparedScene.lighting.normalMapImageBinding,
                { { frameResources.offscreen.normalMapImageView, preparedScene.lighting.normalMapSampler, vk::ImageLayout::eShaderReadOnlyOptimal } });

            gbufferDescriptorSetWrites.WriteCombinedImageDescriptors(
                frameResources.gbufferDescriptorSet,
                &preparedScene.lighting.depthStencilBufferBinding,
                { { frameResources.offscreen.depthBufferView, preparedScene.lighting.depthBufferSampler, vk::ImageLayout::eShaderReadOnlyOptimal } });

            DescriptorSet::Update(gbufferDescriptorSetWrites);

            FramebufferCreateInfo framebufferCreateInfo(preparedScene.lighting.renderPass, swapchainWidth, swapchainHeight, 1);
            framebufferCreateInfo
                .AddAttachment(&preparedScene.lighting.colorAttachment, context->swapchain->GetImageView(i));

            auto[createFramebufferResult, createdFramebuffer] = context->device->CreateFramebuffer(framebufferCreateInfo);
            if (createFramebufferResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.framebuffer = std::move(createdFramebuffer);

            FramebufferCreateInfo offscreenFramebufferCreateInfo(preparedScene.gBuffer.renderPass, swapchainWidth, swapchainHeight, 1);
            offscreenFramebufferCreateInfo
                .AddAttachment(&preparedScene.gBuffer.baseColorAttachment, frameResources.offscreen.baseColorImageView)
                .AddAttachment(&preparedScene.gBuffer.normalMapAttachment, frameResources.offscreen.normalMapImageView)
                .AddAttachment(&preparedScene.gBuffer.depthStencilAttachment, frameResources.offscreen.depthStencilBufferView);

            auto[createOffscreenFramebufferResult, createdOffscreenFramebuffer] = context->device->CreateFramebuffer(offscreenFramebufferCreateInfo);
            if (createOffscreenFramebufferResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.offscreenFramebuffer = std::move(createdOffscreenFramebuffer);

            auto[createFenceResult, createdFence] = context->device->CreateFence();
            if (createFenceResult != vk::Result::eSuccess)
            {
                return { false };
            }

            auto[allocateResult, allocatedBuffers] = frameResources.graphicsCommandPool->AllocateCommandBuffers(2, CommandBufferLevel::Primary);
            if (allocateResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.gBufferCommandBuffer= std::move(allocatedBuffers[0]);
            frameResources.lightingCommandBuffer = std::move(allocatedBuffers[1]);

            frameResources.fence = std::move(createdFence);

            auto[createSemaphoreResult, createdSemaphore] = context->device->CreateSemaphore();
            if (createSemaphoreResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.drawSemaphore = std::move(createdSemaphore);

            auto[createOffscreenSemaphoreResult, createdOffscreenSemaphore] = context->device->CreateSemaphore();
            if (createOffscreenSemaphoreResult != vk::Result::eSuccess)
            {
                return { false };
            }

            frameResources.offscreenSemaphore = std::move(createdOffscreenSemaphore);
        }

        const auto& textures = sceneProperties.textures;

        Vector<SceneV1::Texture*> texturesVector;
        for (const auto& texture : textures)
        {
            texturesVector.push_back(texture);
        }

        TextureUploader textureUploader{ context->device, preparedScene.commandPool };
        auto [uploadTexturesSucceeded, uploadTexturesResult] = textureUploader.UploadTextures(texturesVector);

        if(!uploadTexturesSucceeded)
        {
            return { false };
        }

        const auto& buffers = sceneProperties.buffers;
        for(const auto& buffer : buffers)
        {
            if(!buffer->UploadToDevice(context->device))
            {
                return { false };
            }
        }

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
            if (node->GetCamera() != nullptr)
            {
                HUSKY_ASSERT_MSG(preparedScene.cameraNode == nullptr, "Only one camera node is allowed");
                preparedScene.cameraNode = node;
                PrepareCameraNode(node, preparedScene);
            }
            else if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node, preparedScene);
            }
            else if (node->GetLight() != nullptr)
            {
                PrepareLightNode(node, preparedScene);
            }
            else
            {
                //HUSKY_ASSERT(false);
            }
        }

        for (const auto& material : sceneProperties.materials)
        {
            PrepareMaterial(material, preparedScene);
        }

        PrepareLights(preparedScene);

        return { true, preparedScene };
    }

    void DeferredRenderer::UpdateScene(DeferredPreparedScene& scene)
    {
        Mat4x4 identity = glm::mat4(1.0f);
        for (const auto& node : scene.scene->GetNodes())
        {
            UpdateNode(node, identity, scene);
        }
    }

    void DeferredRenderer::DrawScene(const DeferredPreparedScene& scene)
    {
        auto[acquireResult, index] = context->swapchain->AcquireNextImage(nullptr, context->presentSemaphore);
        HUSKY_ASSERT(acquireResult == vk::Result::eSuccess);

        auto& frameResource = scene.frameResources[index];

        auto &gBufferCmdBuffer = frameResource.gBufferCommandBuffer;

        Array<float32, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        vk::ClearColorValue colorClearValue{ clearColor };
        vk::ClearDepthStencilValue depthStencilClearValue{ maxDepth, 0 };

        Vector<vk::ClearValue> offscreenClearValues = { colorClearValue, colorClearValue, depthStencilClearValue };
        Vector<vk::ClearValue> clearValues = { colorClearValue, depthStencilClearValue };

        int32 framebufferWidth = context->swapchain->GetSwapchainCreateInfo().width;
        int32 framebufferHeight = context->swapchain->GetSwapchainCreateInfo().height;

        gBufferCmdBuffer
            ->Begin()
            ->SetViewport({ 0, 0, (float32)framebufferWidth, (float32)framebufferHeight, 0.0f, 1.0f })
            ->SetScissor({ {0, 0}, {(uint32)framebufferWidth, (uint32)framebufferHeight} })
            ->BindDescriptorSet(scene.gBuffer.pipelineLayout, 0, scene.cameraNode->GetCamera()->GetDescriptorSet())
            ->BeginInlineRenderPass(
                scene.gBuffer.renderPass,
                frameResource.offscreenFramebuffer,
                offscreenClearValues,
                { { 0, 0 },{ (uint32)framebufferWidth, (uint32)framebufferHeight } });

        for (const auto& node : scene.scene->GetNodes())
        {
            DrawNode(node, scene, gBufferCmdBuffer);
        }

        gBufferCmdBuffer
            ->EndRenderPass()
            ->End();

        Submission gBufferSubmission;
        gBufferSubmission.commandBuffers = { gBufferCmdBuffer };
        gBufferSubmission.signalSemaphores = { frameResource.offscreenSemaphore };

        context->device->GetGraphicsQueue()->Submit(gBufferSubmission);

        auto &lightingCmdBuffer = frameResource.lightingCommandBuffer;

        lightingCmdBuffer
            ->Begin()
            ->BindGraphicsPipeline(scene.lighting.pipeline)
            ->SetViewport({ 0, 0, (float32)framebufferWidth, (float32)framebufferHeight, 0.0f, 1.0f })
            ->SetScissor({ { 0, 0 },{ (uint32)framebufferWidth, (uint32)framebufferHeight } })
            ->BindDescriptorSet(scene.lighting.pipelineLayout, 0, scene.cameraNode->GetCamera()->GetDescriptorSet())
            ->BindDescriptorSet(scene.lighting.pipelineLayout, 1, scene.lighting.lightsDescriptorSet)
            ->BindDescriptorSet(scene.lighting.pipelineLayout, 2, frameResource.gbufferDescriptorSet)
            ->BeginInlineRenderPass(
                scene.lighting.renderPass,
                frameResource.framebuffer,
                clearValues,
                { { 0, 0 },{ (uint32)framebufferWidth, (uint32)framebufferHeight } })
            ->BindVertexBuffers({ scene.lighting.fullscreenQuadBuffer }, {0}, 0)
            ->Draw(fullscreenQuadVertices.size(), 1, 0, 0)
            ->EndRenderPass()
            ->End();

        Submission lightingSubmission;
        lightingSubmission.commandBuffers = { lightingCmdBuffer };
        lightingSubmission.fence = frameResource.fence;

        lightingSubmission.waitOperations =
        {
            { context->presentSemaphore, vk::PipelineStageFlagBits::eColorAttachmentOutput },
            { frameResource.offscreenSemaphore, vk::PipelineStageFlagBits::eFragmentShader }
        };

        lightingSubmission.signalSemaphores = { frameResource.drawSemaphore };

        context->device->GetGraphicsQueue()->Submit(lightingSubmission);

        PresentSubmission presentSubmission;
        presentSubmission.index = index;
        presentSubmission.swapchain = context->swapchain;
        presentSubmission.waitSemaphores = { frameResource.drawSemaphore };

        context->device->GetPresentQueue()->Present(presentSubmission);

        frameResource.fence->Wait();
        frameResource.fence->Reset();
        frameResource.graphicsCommandPool->Reset();
    }

    void DeferredRenderer::PrepareCameraNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
        const auto& camera = node->GetCamera();

        auto [createBufferResult, createdBuffer] = context->device->CreateBuffer(
            sizeof(CameraUniformBuffer),
            context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eUniformBuffer,
            true);

        HUSKY_ASSERT(createBufferResult == vk::Result::eSuccess);

        auto[mapMemoryResult, mappedMemory] = createdBuffer->MapMemory(sizeof(CameraUniformBuffer), 0);
        HUSKY_ASSERT(mapMemoryResult == vk::Result::eSuccess);

        auto[createDescriptorSetResult, createdDescriptorSet] = scene.descriptorPool->AllocateDescriptorSet(scene.cameraDescriptorSetLayout);
        HUSKY_ASSERT(createDescriptorSetResult == vk::Result::eSuccess);

        camera->SetUniformBuffer(createdBuffer);
        camera->SetDescriptorSet(createdDescriptorSet);

        DescriptorSetWrites descriptorSetWrites;
        descriptorSetWrites.WriteUniformBufferDescriptors(createdDescriptorSet, &scene.cameraUniformBufferBinding, { createdBuffer });
        DescriptorSet::Update(descriptorSetWrites);
    }

    void DeferredRenderer::PrepareMeshNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
        const auto& mesh = node->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh, scene);
        }

        // TODO sort out node hierarchy
        for (const auto& child : node->GetChildren())
        {
            PrepareMeshNode(node, scene);
        }
    }

    void DeferredRenderer::PrepareLightNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
        const auto& light = node->GetLight();

        if (light!= nullptr)
        {
            PrepareLight(light, scene);
        }

        HUSKY_ASSERT_MSG(node->GetChildren().empty(), "Don't add children to light nodes");
    }

    void DeferredRenderer::PrepareNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene)
    {
    }

    void DeferredRenderer::PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, DeferredPreparedScene& scene)
    {
        for (const auto& primitive : mesh->GetPrimitives())
        {
            RefPtr<Pipeline> pipeline = primitive->GetPipeline();
            if (pipeline == nullptr)
            {
                pipeline = CreateGBufferPipeline(primitive, scene);
                primitive->SetPipeline(pipeline);
            }
        }

        auto[createUniformBufferResult, createdUniformBuffer] = context->device->CreateBuffer(
            sizeof(MeshUniformBuffer),
            context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eUniformBuffer,
            true);

        HUSKY_ASSERT(createUniformBufferResult == vk::Result::eSuccess);

        auto[mapMemoryResult, mappedMemory] = createdUniformBuffer->MapMemory(sizeof(MeshUniformBuffer), 0);
        HUSKY_ASSERT(mapMemoryResult == vk::Result::eSuccess);

        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = scene.gBuffer.descriptorPool->AllocateDescriptorSet(scene.gBuffer.meshDescriptorSetLayout);
        HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);

        mesh->SetUniformBuffer(createdUniformBuffer);
        mesh->SetDescriptorSet(allocatedDescriptorSet);

        DescriptorSetWrites descriptorSetWrites;
        descriptorSetWrites.WriteUniformBufferDescriptors(allocatedDescriptorSet, &scene.gBuffer.meshUniformBufferBinding, { createdUniformBuffer });

        DescriptorSet::Update(descriptorSetWrites);
    }

    void DeferredRenderer::PrepareLight(const RefPtr<SceneV1::Light>& light, DeferredPreparedScene& scene)
    {
        scene.lights.push_back(light);
    }

    void DeferredRenderer::PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& material, DeferredPreparedScene& scene)
    {
        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = scene.gBuffer.descriptorPool->AllocateDescriptorSet(scene.gBuffer.materialDescriptorSetLayout);
        HUSKY_ASSERT(allocateDescriptorSetResult == vk::Result::eSuccess);

        material->SetDescriptorSet(allocatedDescriptorSet);

        DescriptorSetWrites descriptorSetWrites;

        Vector<ImageDescriptorInfo> imageDescriptors;

        if (material->HasBaseColorTexture())
        {
            descriptorSetWrites.WriteCombinedImageDescriptors(
                material->GetDescriptorSet(),
                &scene.gBuffer.baseColorTextureBinding,
                { ToVulkanImageDescriptorInfo(material->metallicRoughness.baseColorTexture) });
        }

        if (material->HasMetallicRoughnessTexture())
        {
            descriptorSetWrites.WriteCombinedImageDescriptors(
                material->GetDescriptorSet(),
                &scene.gBuffer.metallicRoughnessTextureBinding,
                { ToVulkanImageDescriptorInfo(material->metallicRoughness.metallicRoughnessTexture) });
        }

        if (material->HasNormalTexture())
        {
            descriptorSetWrites.WriteCombinedImageDescriptors(
                material->GetDescriptorSet(),
                &scene.gBuffer.normalTextureBinding,
                { ToVulkanImageDescriptorInfo(material->normalTexture) });
        }
        
        if (material->HasOcclusionTexture())
        {
            descriptorSetWrites.WriteCombinedImageDescriptors(
                material->GetDescriptorSet(),
                &scene.gBuffer.occlusionTextureBinding,
                { ToVulkanImageDescriptorInfo(material->occlusionTexture) });
        }
        
        if (material->HasEmissiveTexture())
        {
            descriptorSetWrites.WriteCombinedImageDescriptors(
                material->GetDescriptorSet(),
                &scene.gBuffer.emissiveTextureBinding,
                { ToVulkanImageDescriptorInfo(material->emissiveTexture) });
        }

        DescriptorSet::Update(descriptorSetWrites);
    }

    void DeferredRenderer::PrepareLights(DeferredPreparedScene& scene)
    {
        constexpr int MAX_LIGHTS_COUNT = 8;
        int32 lightsBufferSize = sizeof(LightUniform) * MAX_LIGHTS_COUNT;

        auto[createLightsBufferResult, createdLightsBuffer] = context->device->CreateBuffer(
            lightsBufferSize,
            context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eUniformBuffer, true);

        auto[mapMemoryResult, mappedMemory] = createdLightsBuffer->MapMemory(lightsBufferSize, 0);
        HUSKY_ASSERT(mapMemoryResult == vk::Result::eSuccess);

        memset(mappedMemory, 0, lightsBufferSize);

        HUSKY_ASSERT(createLightsBufferResult == vk::Result::eSuccess);
        scene.lighting.lightsBuffer = createdLightsBuffer;

        auto[createDescriptorSetResult, createdDescriptorSet] = scene.lighting.descriptorPool->AllocateDescriptorSet(scene.lighting.lightsDescriptorSetLayout);
        HUSKY_ASSERT(createDescriptorSetResult == vk::Result::eSuccess);
        scene.lighting.lightsDescriptorSet = createdDescriptorSet;

        DescriptorSetWrites descriptorSetWrites;

        descriptorSetWrites.WriteUniformBufferDescriptors(createdDescriptorSet, &scene.lighting.lightsUniformBufferBinding, { createdLightsBuffer });

        DescriptorSet::Update(descriptorSetWrites);
    }

    void Husky::Render::DeferredRenderer::UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform, DeferredPreparedScene& scene)
    {
        const auto& mesh = node->GetMesh();

        Mat4x4 localTransformMatrix;
        const auto& localTransform = node->GetTransform();
//        if (std::holds_alternative<Mat4x4>(localTransform))
//        {
//            localTransformMatrix = std::get<Mat4x4>(localTransform);
//        }
//        else
//        {
//            const auto& transformProperties = std::get<SceneV1::TransformProperties>(localTransform);
//
//            localTransformMatrix
//                = glm::translate(transformProperties.translation)
//                * glm::toMat4(transformProperties.rotation)
//                * glm::scale(transformProperties.scale);
//        }

        if (localTransform.hasMatrix)
        {
            localTransformMatrix = localTransform.matrix;
        }
        else
        {
            const auto& transformProperties = localTransform.properties;

            localTransformMatrix
                = glm::translate(transformProperties.translation)
                * glm::toMat4(transformProperties.rotation)
                * glm::scale(transformProperties.scale);
        }

        Mat4x4 transform = parentTransform * localTransformMatrix;

        if (mesh != nullptr)
        {
            UpdateMesh(mesh, transform, scene);
        }

        const auto& camera = node->GetCamera();

        if (camera != nullptr)
        {
            UpdateCamera(camera, transform, scene);
        }

        const auto& light = node->GetLight();
        if (light != nullptr)
        {
            UpdateLight(light, transform, scene);
        }

        for (const auto& child : node->GetChildren())
        {
            UpdateNode(child, transform, scene);
        }
    }

    void DeferredRenderer::UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform, DeferredPreparedScene& scene)
    {
        const auto& buffer = mesh->GetUniformBuffer();

        MeshUniformBuffer meshUniformBuffer;
        //meshUniformBuffer.transform = glm::transpose(transform);
        meshUniformBuffer.transform = (transform);

        // TODO template function member in buffer to write updates
        memcpy(buffer->GetMappedMemory(), &meshUniformBuffer, sizeof(MeshUniformBuffer));
    }

    void DeferredRenderer::UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform, DeferredPreparedScene& scene)
    {
        const auto& buffer = camera->GetUniformBuffer();

        camera->SetCameraViewMatrix(transform);

        CameraUniformBuffer cameraUniformBuffer;
        cameraUniformBuffer.view = camera->GetCameraViewMatrix();
        cameraUniformBuffer.projection = camera->GetCameraProjectionMatrix();
        cameraUniformBuffer.position = Vec4{ camera->GetCameraPosition(), 1.0 };

        Vec2 zMinMax;
        auto cameraType = camera->GetCameraType();
        if (cameraType == SceneV1::CameraType::Orthographic)
        {
            auto orthographicCamera = (SceneV1::OrthographicCamera*)camera.Get();
            zMinMax.x = orthographicCamera->GetZNear();
            zMinMax.y = orthographicCamera->GetZFar();
        }
        else if(cameraType == SceneV1::CameraType::Perspective)
        {
            auto perspectiveCamera = (SceneV1::PerspectiveCamera*)camera.Get();
            zMinMax.x = perspectiveCamera->GetZNear();
            zMinMax.y = perspectiveCamera->GetZFar().value_or(Limits<float>::infinity());
        }
        
        cameraUniformBuffer.zMinMax = zMinMax;

        // TODO
        memcpy(buffer->GetMappedMemory(), &cameraUniformBuffer, sizeof(CameraUniformBuffer));
    }

    void DeferredRenderer::UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform, DeferredPreparedScene& scene)
    {
        auto viewTransform = scene.cameraNode->GetCamera()->GetCameraViewMatrix();

        LightUniform lightUniform;

        lightUniform.positionWS = transform * Vec4{ 0.0, 0.0, 0.0, 1.0 };
        // TODO get rid of this workaround
        lightUniform.positionWS.y = -lightUniform.positionWS.y;

        lightUniform.directionWS = transform * Vec4{ light->GetDirection().value_or(Vec3{0, 0, 1}), 0.0 };
        lightUniform.positionVS = viewTransform * lightUniform.positionWS;
        lightUniform.directionVS = viewTransform * lightUniform.directionWS;
        lightUniform.color = Vec4{ light->GetColor().value_or(Vec3{1.0, 1.0, 1.0}), 1.0 };
        lightUniform.enabled = light->IsEnabled() ? 1 : 0;
        lightUniform.type = static_cast<int32>(light->GetType());
        lightUniform.spotlightAngle = light->GetSpotlightAngle().value_or(0.0);
        lightUniform.range = light->GetRange().value_or(0.0);
        lightUniform.intensity = light->GetIntensity();
        lightUniform.enabled = true;

        int32 offset = sizeof(LightUniform)*light->GetIndex();
        memcpy((Byte*)scene.lighting.lightsBuffer->GetMappedMemory() + offset, &lightUniform, sizeof(LightUniform));
    }

    void DeferredRenderer::DrawNode(const RefPtr<SceneV1::Node>& node, const DeferredPreparedScene& scene, CommandBuffer* cmdBuffer)
    {
        const auto& mesh = node->GetMesh();
        if (mesh != nullptr)
        {
            DrawMesh(mesh, scene, cmdBuffer);
        }

        for (const auto& child : node->GetChildren())
        {
            DrawNode(child, scene, cmdBuffer);
        }
    }

    void DeferredRenderer::DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, const DeferredPreparedScene& scene, CommandBuffer* cmdBuffer)
    {
        // TODO descriptor set indices
        cmdBuffer->BindDescriptorSet(scene.gBuffer.pipelineLayout, 1, mesh->GetDescriptorSet());

        for (const auto& primitive : mesh->GetPrimitives())
        {
            if (primitive->GetMaterial()->alphaMode != SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, scene, cmdBuffer);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            if (primitive->GetMaterial()->alphaMode == SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, scene, cmdBuffer);
            }
        }
    }

    void DeferredRenderer::DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, const DeferredPreparedScene& scene, CommandBuffer* cmdBuffer)
    {
        RefPtr<Pipeline> pipeline = primitive->GetPipeline();

        const auto& vertexBuffers = primitive->GetVertexBuffers();

        Vector<DeviceBuffer*> vulkanVertexBuffers;
        Vector<int64> offsets;
        vulkanVertexBuffers.reserve(vertexBuffers.size());
        offsets.reserve(vertexBuffers.size());

        for (const auto& vertexBuffer : vertexBuffers)
        {
            vulkanVertexBuffers.push_back(vertexBuffer.backingBuffer->GetDeviceBuffer());
            offsets.push_back(vertexBuffer.byteOffset);
        }

        HUSKY_ASSERT(primitive->GetIndexBuffer().has_value());
        const auto& indexBuffer = *primitive->GetIndexBuffer();

        const auto& material = primitive->GetMaterial();

        MaterialPushConstants materialPushConstants = GetMaterialPushConstants(material);

        cmdBuffer
            ->BindGraphicsPipeline(pipeline)
            ->BindVertexBuffers(vulkanVertexBuffers, offsets, 0)
            ->BindDescriptorSet(scene.gBuffer.pipelineLayout, 2, primitive->GetMaterial()->GetDescriptorSet())
            ->BindIndexBuffer(
                indexBuffer.backingBuffer->GetDeviceBuffer(),
                ToVulkanIndexType(indexBuffer.indexType),
                0)
            ->PushConstants(scene.gBuffer.pipelineLayout, ShaderStage::Fragment, 0, materialPushConstants)
            ->DrawIndexed(indexBuffer.count, 1, 0, 0, 0);
    }

    ImageDescriptorInfo DeferredRenderer::ToVulkanImageDescriptorInfo(const SceneV1::TextureInfo& textureInfo)
    {
        return ImageDescriptorInfo
        {
            textureInfo.texture->GetDeviceImageView(),
            textureInfo.texture->GetDeviceSampler(),
            vk::ImageLayout::eShaderReadOnlyOptimal
        };
    }

    MaterialPushConstants DeferredRenderer::GetMaterialPushConstants(SceneV1::PbrMaterial * material)
    {
        MaterialPushConstants materialPushConstants;
        materialPushConstants.baseColorFactor = material->metallicRoughness.baseColorFactor;
        materialPushConstants.metallicFactor = material->metallicRoughness.metallicFactor;
        materialPushConstants.roughnessFactor = material->metallicRoughness.roughnessFactor;
        materialPushConstants.normalScale = material->normalTexture.scale;
        materialPushConstants.occlusionStrength = material->occlusionTexture.strength;
        materialPushConstants.emissiveFactor = material->emissiveFactor;
        materialPushConstants.alphaCutoff = material->alphaCutoff;
        return materialPushConstants;
    }

    Vector<Byte> Husky::Render::DeferredRenderer::LoadShaderSource(const FilePath& path)
    {
        FileStream fileStream{ path, FileOpenModes::Read };
        auto fileSize = fileStream.GetSize();
        Vector<Byte> result;
        result.resize(fileSize + 1); // +1 for null termination
        auto bytesRead = fileStream.Read(result.data(), fileSize, sizeof(Byte));
        HUSKY_ASSERT(bytesRead == fileSize);
        return result;
    }

    RefPtr<Pipeline> DeferredRenderer::CreateGBufferPipeline(const RefPtr<SceneV1::Primitive>& primitive, DeferredPreparedScene& scene)
    {
        GraphicsPipelineCreateInfo pipelineState;

        ShaderDefines shaderDefines;

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        pipelineState.vertexInputState.bindingDescriptions.resize(vertexBuffers.size());
        for (int i = 0; i < vertexBuffers.size(); i++)
        {
            const auto& vertexBuffer = vertexBuffers[i];
            auto& bindingDescription = pipelineState.vertexInputState.bindingDescriptions[i];
            bindingDescription.binding = i;
            bindingDescription.stride = vertexBuffer.stride;
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

            // Wtf is this shit
            if (attribute.semantic == SceneV1::AttributeSemantic::Tangent)
            {
                if (attribute.format == Format::R32G32B32A32Sfloat)
                {
                    shaderDefines.AddFlag(ShaderDefine::HasBitangentDirection);
                }
                else if(attribute.format == Format::R32G32B32Sfloat)
                {
                }
                else
                {
                    HUSKY_ASSERT_MSG(false, "Wtf is this tangent");
                }
            }

            shaderDefines.AddFlag(SemanticToFlag.at(attribute.semantic));
        }

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

        // TEST CODE
        pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;

        pipelineState.depthStencilState.depthTestEnable = true;
        pipelineState.depthStencilState.depthWriteEnable = true;
        pipelineState.depthStencilState.depthCompareOp = vk::CompareOp::eLess;

        auto& baseColorAttachmentBlendState = pipelineState.colorBlendState.attachments.emplace_back();
        baseColorAttachmentBlendState.blendEnable = material->alphaMode == SceneV1::AlphaMode::Blend;
        if (baseColorAttachmentBlendState.blendEnable)
        {
            baseColorAttachmentBlendState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            baseColorAttachmentBlendState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            baseColorAttachmentBlendState.colorBlendOp = vk::BlendOp::eAdd;
            baseColorAttachmentBlendState.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            baseColorAttachmentBlendState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            baseColorAttachmentBlendState.alphaBlendOp = vk::BlendOp::eAdd;
        }

        auto& normalMapAttachmentBlendState = pipelineState.colorBlendState.attachments.emplace_back();
        normalMapAttachmentBlendState.blendEnable = false;

        pipelineState.renderPass = scene.gBuffer.renderPass;
        pipelineState.layout = scene.gBuffer.pipelineLayout;

        pipelineState.viewportState.viewports.emplace_back();
        pipelineState.viewportState.scissors.emplace_back();

        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eScissor);
        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eViewport);

        if (material->HasBaseColorTexture())
        {
            shaderDefines.AddFlag(ShaderDefine::HasBaseColorTexture);
        }

        if (material->HasMetallicRoughnessTexture())
        {
            shaderDefines.AddFlag(ShaderDefine::HasMetallicRoughnessTexture);
        }

        if (material->HasNormalTexture())
        {
            shaderDefines.AddFlag(ShaderDefine::HasNormalTexture);
        }

        if (material->HasOcclusionTexture())
        {
            shaderDefines.AddFlag(ShaderDefine::HasOcclusionTexture);
        }

        if (material->HasEmissiveTexture())
        {
            shaderDefines.AddFlag(ShaderDefine::HasEmissiveTexture);
        }

        if (material->alphaMode == SceneV1::AlphaMode::Mask)
        {
            shaderDefines.AddFlag(ShaderDefine::AlphaMask);
        }

        auto[vertexShaderCreated, createdVertexShader] = CreateShader(
            ShaderStage::Vertex,
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.vert",
            shaderDefines);

        if (!vertexShaderCreated)
        {
            HUSKY_ASSERT(false);
        }

        auto vertexShader = std::move(createdVertexShader);

        auto[fragmentShaderCreated, createdFragmentShader] = CreateShader(
            ShaderStage::Fragment,
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.frag",
            shaderDefines);

        if (!fragmentShaderCreated)
        {
            HUSKY_ASSERT(false);
        }

        auto fragmentShader = std::move(createdFragmentShader);

        auto& vertexShaderStage = pipelineState.shaderStages.emplace_back();
        vertexShaderStage.name = "main";
        vertexShaderStage.shaderModule = vertexShader.module;
        vertexShaderStage.stage = ShaderStage::Vertex;

        auto& fragmentShaderStage = pipelineState.shaderStages.emplace_back();
        fragmentShaderStage.name = "main";
        fragmentShaderStage.shaderModule = fragmentShader.module;
        fragmentShaderStage.stage = ShaderStage::Fragment;

        // Retain shader modules
        primitive->AddShaderModule(vertexShader.module);
        primitive->AddShaderModule(fragmentShader.module);

        auto[createPipelineResult, createdPipeline] = context->device->CreateGraphicsPipeline(pipelineState);
        if (createPipelineResult != vk::Result::eSuccess)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    RefPtr<Pipeline> Husky::Render::DeferredRenderer::CreateLightingPipeline(const LightingPassResources& lighting)
    {
        GraphicsPipelineCreateInfo pipelineState;

        auto& vertexShaderStage = pipelineState.shaderStages.emplace_back();
        vertexShaderStage.name = "main";
        vertexShaderStage.shaderModule = lighting.vertexShader.module;
        vertexShaderStage.stage = ShaderStage::Vertex;

        auto& fragmentShaderStage = pipelineState.shaderStages.emplace_back();
        fragmentShaderStage.name = "main";
        fragmentShaderStage.shaderModule = lighting.fragmentShader.module;
        fragmentShaderStage.stage = ShaderStage::Fragment;

        auto& bindingDescription = pipelineState.vertexInputState.bindingDescriptions.emplace_back();
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(QuadVertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        auto& positionAttributeDescription = pipelineState.vertexInputState.attributeDescriptions.emplace_back();
        positionAttributeDescription.location = 0;
        positionAttributeDescription.binding = bindingDescription.binding;
        positionAttributeDescription.format = vk::Format::eR32G32B32Sfloat;
        positionAttributeDescription.offset = offsetof(QuadVertex, position);

        auto& texCoordAttributeDescription = pipelineState.vertexInputState.attributeDescriptions.emplace_back();
        texCoordAttributeDescription.location = 1;
        texCoordAttributeDescription.binding = bindingDescription.binding;
        texCoordAttributeDescription.format = vk::Format::eR32G32Sfloat;
        texCoordAttributeDescription.offset = offsetof(QuadVertex, texCoord);

        pipelineState.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
        pipelineState.rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;

        pipelineState.depthStencilState.depthTestEnable = false;
        pipelineState.depthStencilState.depthWriteEnable = false;

        auto& attachmentBlendState = pipelineState.colorBlendState.attachments.emplace_back();
        attachmentBlendState.blendEnable = false;

        pipelineState.renderPass = lighting.renderPass;
        pipelineState.layout = lighting.pipelineLayout;

        pipelineState.viewportState.viewports.emplace_back();
        pipelineState.viewportState.scissors.emplace_back();

        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eScissor);
        pipelineState.dynamicState.dynamicStates.push_back(vk::DynamicState::eViewport);

        auto[createPipelineResult, createdPipeline] = context->device->CreateGraphicsPipeline(pipelineState);
        if (createPipelineResult != vk::Result::eSuccess)
        {
            HUSKY_ASSERT(false);
        }

        return createdPipeline;
    }

    Vector<const char8*> DeferredRenderer::GetRequiredDeviceExtensionNames() const
    {
        Vector<const char8*> requiredExtensionNames;

        requiredExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        return requiredExtensionNames;
    }

    ResultValue<bool, GBufferPassResources> DeferredRenderer::PrepareGBufferPassResources(DeferredPreparedScene& scene)
    {
        GBufferPassResources resources;

        const auto& sceneProperties = scene.scene->GetSceneProperties();

        const int32 texturesPerMaterial = 5;

        int32 textureDescriptorCount = sceneProperties.materials.size() * texturesPerMaterial;
        int32 perMeshUBOCount = sceneProperties.meshes.size();

        // one set per material, one set per mesh
        int32 maxDescriptorSets = sceneProperties.materials.size() + sceneProperties.meshes.size();

        UnorderedMap<vk::DescriptorType, int32> descriptorCount =
        {
            { vk::DescriptorType::eCombinedImageSampler, textureDescriptorCount + 1 },
            { vk::DescriptorType::eUniformBuffer, perMeshUBOCount + 1 },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(maxDescriptorSets, descriptorCount);
        if (createDescriptorPoolResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.descriptorPool = std::move(createdDescriptorPool);

        resources.meshUniformBufferBinding
            .OfType(vk::DescriptorType::eUniformBuffer)
            .AtStages(ShaderStage::Vertex);

        resources.baseColorTextureBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        resources.metallicRoughnessTextureBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        resources.normalTextureBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        resources.occlusionTextureBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        resources.emissiveTextureBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
        meshDescriptorSetLayoutCreateInfo
            .AddBinding(&resources.meshUniformBufferBinding);

        DescriptorSetLayoutCreateInfo materialDescriptorSetLayoutCreateInfo;
        materialDescriptorSetLayoutCreateInfo
            .AddBinding(&resources.baseColorTextureBinding)
            .AddBinding(&resources.metallicRoughnessTextureBinding)
            .AddBinding(&resources.normalTextureBinding)
            .AddBinding(&resources.occlusionTextureBinding)
            .AddBinding(&resources.emissiveTextureBinding);

        auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(meshDescriptorSetLayoutCreateInfo);
        if (createMeshDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.meshDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);

        auto[createMaterialDescriptorSetLayoutResult, createdMaterialDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(materialDescriptorSetLayoutCreateInfo);
        if (createMaterialDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.materialDescriptorSetLayout = std::move(createdMaterialDescriptorSetLayout);

        // TODO check push constant size limit

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .WithNSetLayouts(3)
            .AddSetLayout(scene.cameraDescriptorSetLayout)
            .AddSetLayout(resources.meshDescriptorSetLayout)
            .AddSetLayout(resources.materialDescriptorSetLayout)
            .WithNPushConstantRanges(1)
            .AddPushConstantRange(ShaderStage::Fragment, sizeof(MaterialPushConstants));

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.pipelineLayout = std::move(createdPipelineLayout);

        resources.baseColorAttachment
            .SetFormat(Format::B8G8R8A8Unorm)
            .SetInitialLayout(vk::ImageLayout::eUndefined)
            .SetLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStoreOp(vk::AttachmentStoreOp::eStore)
            .SetSampleCount(SampleCount::e1)
            .SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

        resources.normalMapAttachment
            .SetFormat(Format::B8G8R8A8Unorm)
            .SetInitialLayout(vk::ImageLayout::eUndefined)
            .SetLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStoreOp(vk::AttachmentStoreOp::eStore)
            .SetSampleCount(SampleCount::e1)
            .SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

        resources.depthStencilAttachment
            .SetFormat(depthStencilFormat)
            .SetInitialLayout(vk::ImageLayout::eUndefined)
            .SetLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStoreOp(vk::AttachmentStoreOp::eStore)
            .SetStencilLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStencilStoreOp(vk::AttachmentStoreOp::eStore)
            .SetSampleCount(SampleCount::e1)
            .SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        SubpassDescription subpassDescription;
        subpassDescription
            .WithNColorAttachments(OffscreenImagesCount - 1) // -1 for depth/stencil attachment
            .AddColorAttachment(&resources.baseColorAttachment, vk::ImageLayout::eColorAttachmentOptimal)
            .AddColorAttachment(&resources.normalMapAttachment, vk::ImageLayout::eColorAttachmentOptimal)
            .WithDepthStencilAttachment(&resources.depthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
            .AddAttachment(&resources.baseColorAttachment)
            .AddAttachment(&resources.normalMapAttachment)
            .AddAttachment(&resources.depthStencilAttachment)
            .AddSubpass(std::move(subpassDescription));

        auto[createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
        if (createRenderPassResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.renderPass = std::move(createdRenderPass);

        return { true, resources };
    }

    ResultValue<bool, LightingPassResources> DeferredRenderer::PrepareLightingPassResources(DeferredPreparedScene& scene)
    {
        LightingPassResources resources;
        int32 swapchainLength = context->swapchain->GetSwapchainCreateInfo().imageCount;

        int32 maxDescriptorSets = 1 + swapchainLength + 1;

        UnorderedMap<vk::DescriptorType, int32> descriptorCount =
        {
            { vk::DescriptorType::eUniformBuffer, 1 },
            { vk::DescriptorType::eCombinedImageSampler, swapchainLength * OffscreenImagesCount + 1 },
        };

        ShaderDefines shaderDefines;

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(maxDescriptorSets, descriptorCount);
        if (createDescriptorPoolResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.descriptorPool = std::move(createdDescriptorPool);

        auto[vertexShaderCreated, createdVertexShader] = CreateShader(
            ShaderStage::Vertex,
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.vert",
            shaderDefines);

        if (!vertexShaderCreated)
        {
            return { false };
        }

        resources.vertexShader = std::move(createdVertexShader);

        auto[fragmentShaderCreated, createdFragmentShader] = CreateShader(
            ShaderStage::Fragment,
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.frag",
            shaderDefines);

        if (!fragmentShaderCreated)
        {
            return { false };
        }

        resources.fragmentShader = std::move(createdFragmentShader);

        resources.lightsUniformBufferBinding
            .OfType(vk::DescriptorType::eUniformBuffer)
            .AtStages(ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
        lightsDescriptorSetLayoutCreateInfo
            .AddBinding(&resources.lightsUniformBufferBinding);

        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
        if (createLightsDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.lightsDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);

        resources.baseColorImageBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        resources.normalMapImageBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        resources.depthStencilBufferBinding
            .OfType(vk::DescriptorType::eCombinedImageSampler)
            .AtStages(ShaderStage::Fragment);

        DescriptorSetLayoutCreateInfo gbufferDescriptorSetLayoutCreateInfo;
        gbufferDescriptorSetLayoutCreateInfo
            .AddBinding(&resources.baseColorImageBinding)
            .AddBinding(&resources.normalMapImageBinding)
            .AddBinding(&resources.depthStencilBufferBinding);

        auto[createGBufferDescriptorSetLayoutResult, createdGBufferDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferDescriptorSetLayoutCreateInfo);
        if (createGBufferDescriptorSetLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.gbufferDescriptorSetLayout = std::move(createdGBufferDescriptorSetLayout);

        vk::SamplerCreateInfo samplerCreateInfo;
        
        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createBaseColorSamplerResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.baseColorSampler = std::move(createdBaseColorSampler);

        auto[createNormalMapSamplerResult, createdNormalMapSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createNormalMapSamplerResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.normalMapSampler = std::move(createdNormalMapSampler);

        auto[createDepthSamplerResult, createdDepthSampler] = context->device->CreateSampler(samplerCreateInfo);
        if (createDepthSamplerResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.depthBufferSampler = std::move(createdDepthSampler);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .WithNSetLayouts(3)
            .AddSetLayout(scene.cameraDescriptorSetLayout)
            .AddSetLayout(resources.lightsDescriptorSetLayout)
            .AddSetLayout(resources.gbufferDescriptorSetLayout);

        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.pipelineLayout = std::move(createdPipelineLayout);

        resources.colorAttachment
            .SetFormat(context->swapchain->GetFormat())
            .SetInitialLayout(vk::ImageLayout::eUndefined)
            .SetLoadOp(vk::AttachmentLoadOp::eClear)
            .SetStoreOp(vk::AttachmentStoreOp::eStore)
            .SetSampleCount(SampleCount::e1)
            .SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        SubpassDescription subpass;
        subpass
            .AddColorAttachment(&resources.colorAttachment, vk::ImageLayout::eColorAttachmentOptimal);

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
            .AddAttachment(&resources.colorAttachment)
            .AddSubpass(std::move(subpass));

        auto[createLightingPassResult, createdLightingPass] = context->device->CreateRenderPass(renderPassCreateInfo);
        if (createLightingPassResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.renderPass = std::move(createdLightingPass);

        // TODO result
        resources.pipeline = CreateLightingPipeline(resources);

        int32 quadBufferSize = fullscreenQuadVertices.size() * sizeof(QuadVertex);

        auto[createQuadBufferResult, createdQuadBuffer] = context->device->CreateBuffer(
            quadBufferSize,
            context->device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eVertexBuffer,
            true);

        if (createQuadBufferResult != vk::Result::eSuccess)
        {
            return { false };
        }

        resources.fullscreenQuadBuffer = std::move(createdQuadBuffer);

        auto [mapMemoryResult, mappedMemory] = resources.fullscreenQuadBuffer->MapMemory(quadBufferSize, 0);
        if (mapMemoryResult != vk::Result::eSuccess)
        {
            return { false };
        }

        memcpy(mappedMemory, fullscreenQuadVertices.data(), quadBufferSize);

        return { true, resources };
    }

    ResultValue<bool, OffscreenImages> DeferredRenderer::CreateOffscreenImages()
    {
        OffscreenImages images;

        auto indices = context->device->GetQueueIndices();
        const auto& swapchainCreateInfo = context->swapchain->GetSwapchainCreateInfo();
        vk::Extent3D imageExtent = { (uint32)swapchainCreateInfo.width, (uint32)swapchainCreateInfo.height, 1 };

        vk::ImageCreateInfo baseColorImageCreateInfo;
        baseColorImageCreateInfo.setFormat(vk::Format::eB8G8R8A8Unorm);
        baseColorImageCreateInfo.setArrayLayers(1);
        baseColorImageCreateInfo.setImageType(vk::ImageType::e2D);
        baseColorImageCreateInfo.setExtent(imageExtent);
        baseColorImageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
        baseColorImageCreateInfo.setMipLevels(1);
        baseColorImageCreateInfo.setQueueFamilyIndexCount(1);
        baseColorImageCreateInfo.setPQueueFamilyIndices(&indices->graphicsQueueFamilyIndex);
        baseColorImageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
        baseColorImageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
        baseColorImageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
        baseColorImageCreateInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc);

        auto[createBaseColorImageResult, createdBaseColorImage] = context->device->CreateImage(baseColorImageCreateInfo);
        if (createBaseColorImageResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.baseColorImage = std::move(createdBaseColorImage);

        auto[createBaseColorViewResult, createdBaseColorView] = context->device->CreateImageView(images.baseColorImage);
        if (createBaseColorViewResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.baseColorImageView = std::move(createdBaseColorView);

        // Normal map
        vk::ImageCreateInfo normalMapImageCreateInfo;
        normalMapImageCreateInfo.setFormat(vk::Format::eB8G8R8A8Unorm);
        normalMapImageCreateInfo.setArrayLayers(1);
        normalMapImageCreateInfo.setImageType(vk::ImageType::e2D);
        normalMapImageCreateInfo.setExtent(imageExtent);
        normalMapImageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
        normalMapImageCreateInfo.setMipLevels(1);
        normalMapImageCreateInfo.setQueueFamilyIndexCount(1);
        normalMapImageCreateInfo.setPQueueFamilyIndices(&indices->graphicsQueueFamilyIndex);
        normalMapImageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
        normalMapImageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
        normalMapImageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
        normalMapImageCreateInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc);

        auto[createNormapMapImageResult, createdNormapMapImage] = context->device->CreateImage(normalMapImageCreateInfo);
        if (createNormapMapImageResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.normalMapImage = std::move(createdNormapMapImage);

        auto[createNormapMapViewResult, createdNormapMapView] = context->device->CreateImageView(images.normalMapImage);
        if (createNormapMapViewResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.normalMapImageView = std::move(createdNormapMapView);

        // Depth Stencil
        vk::ImageCreateInfo depthBufferCreateInfo;
        depthBufferCreateInfo.setFormat(ToVulkanFormat(depthStencilFormat));
        depthBufferCreateInfo.setArrayLayers(1);
        depthBufferCreateInfo.setImageType(vk::ImageType::e2D);
        depthBufferCreateInfo.setExtent(imageExtent);
        depthBufferCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
        depthBufferCreateInfo.setMipLevels(1);
        depthBufferCreateInfo.setQueueFamilyIndexCount(1);
        depthBufferCreateInfo.setPQueueFamilyIndices(&indices->graphicsQueueFamilyIndex);
        depthBufferCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
        depthBufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
        depthBufferCreateInfo.setTiling(vk::ImageTiling::eOptimal);
        depthBufferCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc);

        auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = context->device->CreateImage(depthBufferCreateInfo);
        if (createDepthStencilBufferResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.depthStencilBuffer = std::move(createdDepthStencilBuffer);

        auto depthStencilBufferViewCreateInfo = context->device->GetDefaultImageViewCreateInfo(images.depthStencilBuffer);

        auto[createDepthStencilBufferViewResult, createdDepthStencilBufferView] = context->device->CreateImageView(images.depthStencilBuffer, depthStencilBufferViewCreateInfo);
        if (createDepthStencilBufferViewResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.depthStencilBufferView = std::move(createdDepthStencilBufferView);

        depthStencilBufferViewCreateInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

        auto[createDepthBufferViewResult, createdDepthBufferView] = context->device->CreateImageView(images.depthStencilBuffer, depthStencilBufferViewCreateInfo);
        if (createDepthBufferViewResult != vk::Result::eSuccess)
        {
            return { false };
        }

        images.depthBufferView = std::move(createdDepthBufferView);

        return { true, images };
    }

    ResultValue<bool, Shader> DeferredRenderer::CreateShader(ShaderStage stage, const String& path, const ShaderDefines& defines)
    {
        Shader shader;

        auto shaderSource = LoadShaderSource(path);

        auto shaderCompiled = context->shaderCompiler.TryCompileShader(
            stage,
            shaderSource,
            shader.bytecode,
            defines.defines);

        if (!shaderCompiled)
        {
            return { false };
        }

        auto[createShaderModuleResult, createdShaderModule] = context->device->CreateShaderModule(
            shader.bytecode.data(),
            shader.bytecode.size() * sizeof(uint32)
        );

        shader.module = std::move(createdShaderModule);

        return { true, shader };
    }
}
