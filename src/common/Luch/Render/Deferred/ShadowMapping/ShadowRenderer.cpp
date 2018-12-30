#include <Luch/Render/Deferred/ShadowMapping/ShadowRenderer.h>

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
#include <Luch/Graphics/Swapchain.h>
#include <Luch/Graphics/FrameBuffer.h>
#include <Luch/Graphics/SwapchainInfo.h>
#include <Luch/Graphics/PipelineState.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>
#include <Luch/Graphics/PipelineStateCreateInfo.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Light.h>

#include <Luch/Render/RenderContext.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/SharedBuffer.h>
#include <cstring>

namespace Luch::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    const String ShadowRenderer::RendererName{"Shadow"};

    ShadowRenderer::ShadowRenderer() = default;

    bool ShadowRenderer::Initialize()
    {
        auto[prepareResourcesResult, preparedResources] = PrepareShadowMappingPassResources();
        if(prepareResourcesResult)
        {
            resources = std::move(preparedResources);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool ShadowRenderer::Deinitialize()
    {
        resources.reset();
        return true;
    }

    void ShadowRenderer::PrepareScene(SceneV1::Scene* scene)
    {
        const auto& nodes = scene->GetNodes();

        for (const auto& node : nodes)
        {
            if(node->GetMesh() != nullptr)
            {
                PrepareMeshNode(node);
            }
        }
    }

    void ShadowRenderer::UpdateScene(SceneV1::Scene* scene)
    {
        for (const auto& node : scene->GetNodes())
        {
            UpdateNode(node);
        }
    }

    const ShadowRenderer::ShadowMaps& ShadowRenderer::DrawShadows(
        SceneV1::Scene* scene,
        const RefPtrVector<SceneV1::Node>& lightNodes)
    {
        auto [createCommandListResult, commandList] = resources->commandPool->AllocateGraphicsCommandList();
        LUCH_ASSERT(createCommandListResult == GraphicsResult::Success);

        shadowMaps.clear();
        commandList->Begin();

        for(const auto& lightNode : lightNodes)
        {
            const auto& light = lightNode->GetLight();
            LUCH_ASSERT(light != nullptr);

            if(light->IsShadowEnabled())
            {
                shadowMaps[light] = DrawSceneForLight(light, lightNode->GetWorldTransform(), scene, commandList);
            }
        }

        commandList->End();

        context->commandQueue->Submit(commandList);

        return shadowMaps;
    }

    RefPtrVector<Texture> ShadowRenderer::DrawSceneForLight(
        SceneV1::Light* light,
        Mat4x4 transform,
        SceneV1::Scene* scene,
        GraphicsCommandList* commandList)
    {
        RefPtrVector<Texture> shadowMaps;

        switch(light->GetType())
        {
        case SceneV1::LightType::Point:
        {
            shadowMaps.resize(6);
            auto it = pointLightCameras.find(light);
            if(it == pointLightCameras.end())
            {
                RefPtrArray<SceneV1::PerspectiveCamera, 6> cameras;
                for(int32 i = 0; i < 6; i++)
                {
                    auto newCamera = MakeRef<SceneV1::PerspectiveCamera>();
                    PrepareCamera(newCamera);
                    cameras[i] = newCamera;
                }
                it = pointLightCameras.insert(std::make_pair(light, cameras)).first;
            }

            for(int32 i = 0; i < 6; i++)
            {
                const auto& camera = it->second[i];
                UpdatePointLightCamera(light, transform, camera, i);
                shadowMaps[i] = DrawSceneForCamera(camera, scene, commandList);
            }
            break;
        }
        case SceneV1::LightType::Spot:
        {
            auto it = spotLightCameras.find(light);
            if(it == spotLightCameras.end())
            {
                auto newCamera = MakeRef<SceneV1::PerspectiveCamera>();
                it = spotLightCameras.insert(std::make_pair(light, newCamera)).first;
                PrepareCamera(newCamera);
            }

            const auto& camera = it->second;
            UpdateSpotLightCamera(light, transform, camera);
            shadowMaps.emplace_back(DrawSceneForCamera(camera, scene, commandList));
            break;
        }
        case SceneV1::LightType::Directional:
        {
            LUCH_ASSERT_MSG(false, "Not implemented");

            auto it = directionalLightCameras.find(light);
            if(it == directionalLightCameras.end())
            {
                auto newCamera = MakeRef<SceneV1::OrthographicCamera>();
                it = directionalLightCameras.insert(std::make_pair(light, newCamera)).first;
                PrepareCamera(newCamera);
            }

            const auto& camera = it->second;
            UpdateDirectionalLightCamera(light, transform, camera);
            shadowMaps.emplace_back(DrawSceneForCamera(camera, scene, commandList));
            break;
        }
        default:
            break;
        }

        return shadowMaps;
    }

    RefPtr<Texture> ShadowRenderer::DrawSceneForCamera(
        SceneV1::Camera* camera,
        SceneV1::Scene* scene,
        GraphicsCommandList* commandList)
    {
        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.width = options.shadowMapSize;
        textureCreateInfo.height = options.shadowMapSize;
        textureCreateInfo.format = options.shadowMapFormat;
        textureCreateInfo.usage =
            TextureUsageFlags::TransferSource
          | TextureUsageFlags::ShaderRead
          | TextureUsageFlags::DepthStencilAttachment;

        auto [createDepthBufferResult, createdDepthBuffer] = context->device->CreateTexture(textureCreateInfo);
        if(createDepthBufferResult != GraphicsResult::Success)
        {
            return nullptr;
        }

        FrameBufferCreateInfo frameBufferCreateInfo;
        frameBufferCreateInfo.depthStencilTexture = createdDepthBuffer;
        frameBufferCreateInfo.renderPass = resources->renderPass;

        auto [createFrameBufferResult, frameBuffer] = context->device->CreateFrameBuffer(frameBufferCreateInfo);
        LUCH_ASSERT(createFrameBufferResult == GraphicsResult::Success);

        commandList->BeginRenderPass(frameBuffer);
        commandList->BindPipelineState(resources->pipelineState);

        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            resources->pipelineLayout,
            camera->GetDescriptorSet(RendererName));

        for(const auto& node : scene->GetNodes())
        {
            DrawNode(node, commandList);
        }

        commandList->EndRenderPass();

        return createdDepthBuffer;
    }

    void ShadowRenderer::DrawNode(
        SceneV1::Node* node,
        GraphicsCommandList* commandList)
    {
        const auto& mesh = node->GetMesh();
        if (mesh != nullptr)
        {
            DrawMesh(mesh, commandList);
        }

        for (const auto& child : node->GetChildren())
        {
            DrawNode(child, commandList);
        }
    }

    void ShadowRenderer::DrawMesh(
        SceneV1::Mesh* mesh,
        GraphicsCommandList* commandList)
    {
        commandList->BindBufferDescriptorSet(
            ShaderStage::Vertex,
            resources->pipelineLayout,
            mesh->GetBufferDescriptorSet(RendererName));

        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->GetProperties().alphaMode != SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, commandList);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, commandList);
            }
        }
    }

    void ShadowRenderer::DrawPrimitive(
        SceneV1::Primitive* primitive,
        GraphicsCommandList* commandList)
    {
        const auto& vertexBuffers = primitive->GetVertexBuffers();

        Vector<Buffer*> graphicsVertexBuffers;
        Vector<int32> offsets;
        graphicsVertexBuffers.reserve(vertexBuffers.size());
        offsets.reserve(vertexBuffers.size());

        for (const auto& vertexBuffer : vertexBuffers)
        {
            graphicsVertexBuffers.push_back(vertexBuffer.backingBuffer->GetDeviceBuffer());
            offsets.push_back(vertexBuffer.byteOffset);
        }

        LUCH_ASSERT(primitive->GetIndexBuffer().has_value());
        const auto& indexBuffer = *primitive->GetIndexBuffer();

        commandList->BindVertexBuffers(graphicsVertexBuffers, offsets, 0);

        commandList->BindIndexBuffer(
                indexBuffer.backingBuffer->GetDeviceBuffer(),
                indexBuffer.indexType,
                indexBuffer.byteOffset);

        commandList->DrawIndexedInstanced(indexBuffer.count, 0, 1, 0);
    }

    void ShadowRenderer::PrepareCamera(SceneV1::Camera* camera)
    {
        auto[createVertexDescriptorSetResult, vertexDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->cameraBufferSetLayout);
        LUCH_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);

        camera->SetDescriptorSet(RendererName, vertexDescriptorSet);
    }

    void ShadowRenderer::PrepareMesh(SceneV1::Mesh* mesh)
    {
        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->meshBufferSetLayout);

        LUCH_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);

        mesh->SetBufferDescriptorSet(RendererName, allocatedDescriptorSet);
    }

    void ShadowRenderer::PrepareMeshNode(SceneV1::Node* meshNode)
    {
        const auto& mesh = meshNode->GetMesh();

        if (mesh != nullptr)
        {
            PrepareMesh(mesh);
        }

        // TODO sort out node hierarchy
        for (const auto& child : meshNode->GetChildren())
        {
            PrepareMeshNode(child);
        }
    }

    void ShadowRenderer::UpdateNode(SceneV1::Node* node)
    {
        auto worldTransform = node->GetWorldTransform();
        const auto& mesh = node->GetMesh();
        if(mesh != nullptr)
        {
            UpdateMesh(mesh, worldTransform);
        }
        for (const auto& child : node->GetChildren())
        {
            UpdateNode(child);
        }
    }

    void ShadowRenderer::UpdateMesh(SceneV1::Mesh* mesh, Mat4x4 transform)
    {
        MeshUniform meshUniform;
        meshUniform.transform = transform;
        meshUniform.inverseTransform = glm::inverse(transform);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MeshUniform), 16);

        memcpy(suballocation.offsetMemory, &meshUniform, sizeof(MeshUniform));

        auto descriptorSet = mesh->GetBufferDescriptorSet(RendererName);

        descriptorSet->WriteUniformBuffer(
            resources->meshUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        descriptorSet->Update();
    }

    void ShadowRenderer::UpdateSpotLightCamera(
        SceneV1::Light* light,
        Mat4x4 transform,
        SceneV1::PerspectiveCamera* camera)
    {;
        LUCH_ASSERT(light->GetOuterConeAngle().has_value());
        LUCH_ASSERT(light->GetRange().has_value());

        camera->SetYFOV(light->GetOuterConeAngle().value());
        camera->SetAspectRatio(1);
        camera->SetZNear(0.1);
        camera->SetZFar(light->GetRange().value());

        UpdateCamera(camera, transform);
    }

    void ShadowRenderer::UpdateDirectionalLightCamera(
        SceneV1::Light* light,
        Mat4x4 transform,
        SceneV1::OrthographicCamera* camera)
    {
        LUCH_ASSERT_MSG(false, "Not implemented");

        UpdateCamera(camera, transform);
    }

    void ShadowRenderer::UpdatePointLightCamera(
        SceneV1::Light* light,
        Mat4x4 transform,
        SceneV1::PerspectiveCamera* camera,
        int32 index)
    {
        // TODO

        LUCH_ASSERT(light->GetRange().has_value());

        camera->SetYFOV(glm::pi<float32>() / 2);
        camera->SetAspectRatio(1);
        camera->SetZNear(0.1);
        camera->SetZFar(light->GetRange().value());

        UpdateCamera(camera, transform);
    }

    void ShadowRenderer::UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform)
    {
        auto cameraUniform = RenderUtils::GetCameraUniform(camera, transform);
        auto descriptorSet = camera->GetDescriptorSet(RendererName);

        // TODO
        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(CameraUniform), 16);

        descriptorSet->WriteUniformBuffer(
            resources->cameraUniformBufferBinding,
            suballocation.buffer,
            suballocation.offset);

        descriptorSet->Update();

        memcpy(suballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));
    }

    ResultValue<bool, UniquePtr<ShadowMappingPassResources>> ShadowRenderer::PrepareShadowMappingPassResources()
    {
        UniquePtr<ShadowMappingPassResources> resources = MakeUnique<ShadowMappingPassResources>();

        DepthStencilAttachment depthStencilAttachment;
        depthStencilAttachment.format = options.shadowMapFormat;
        depthStencilAttachment.depthClearValue = 1.0f;
        depthStencilAttachment.depthLoadOperation = AttachmentLoadOperation::Clear;
        depthStencilAttachment.depthStoreOperation = AttachmentStoreOperation::Store;

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.depthStencilAttachment = depthStencilAttachment;

        auto[createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
        if(createRenderPassResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->renderPass = std::move(createdRenderPass);

        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        // two sets per material, one set per mesh
        descriptorPoolCreateInfo.maxDescriptorSets = MaxDescriptorSetCount;

        descriptorPoolCreateInfo.descriptorCount =
        {
            { ResourceType::UniformBuffer, MaxDescriptorCount },
        };

        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
            descriptorPoolCreateInfo);

        if (createDescriptorPoolResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->descriptorPool = std::move(createdDescriptorPool);

        resources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);
        resources->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        DescriptorSetLayoutCreateInfo cameraBufferSetLayoutCreateInfo;
        cameraBufferSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources->cameraUniformBufferBinding);

        auto [cameraBufferSetLayoutCreateResult, createdCameraBufferSetLayout] = context->device->CreateDescriptorSetLayout(
            cameraBufferSetLayoutCreateInfo);
        if(cameraBufferSetLayoutCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->cameraBufferSetLayout = std::move(createdCameraBufferSetLayout);

        DescriptorSetLayoutCreateInfo meshBufferSetLayoutCreateInfo;
        meshBufferSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources->meshUniformBufferBinding);

        auto [meshBufferSetLayoutCreateResult, createdMeshBufferSetLayout] = context->device->CreateDescriptorSetLayout(
            meshBufferSetLayoutCreateInfo);
        if(meshBufferSetLayoutCreateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->meshBufferSetLayout = std::move(createdMeshBufferSetLayout);

        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo
            .AddSetLayout(ShaderStage::Vertex, resources->cameraBufferSetLayout)
            .AddSetLayout(ShaderStage::Vertex, resources->meshBufferSetLayout);

        auto [createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
        if(createPipelineLayoutResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->pipelineLayout = std::move(createdPipelineLayout);

        VertexInputAttributeDescription positionAttribute;
        positionAttribute.format = Format::R32G32B32A32Sfloat;

        VertexInputBindingDescription vertexBufferBinding;
        vertexBufferBinding.stride = sizeof(Vertex);

        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
            context->device,
#if _WIN32
            "C:\\Development\\Luch\\src\\Luch\\Render\\Shaders\\Deferred\\todo.vert",
#endif
#if __APPLE__
    #if LUCH_USE_METAL
            "/Users/spo1ler/Development/Luch/src/Metal/Luch/Render/Shaders/Deferred/shadowmap_vp.metal",
    #elif LUCH_USE_VULKAN
            "/Users/spo1ler/Development/Luch/src/Vulkan/Luch/Render/Shaders/Deferred/todo.vert",
    #else
            "",
    #endif
#endif
#if __linux__
    "",
#endif
            {});

        if(!vertexShaderLibraryCreated)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        auto [createVertexProgramResult, createdVertexProgram] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "vp_main");

        if(createVertexProgramResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        PipelineStateCreateInfo pipelineStateCreateInfo;

        pipelineStateCreateInfo.inputAssembler.attributes.push_back(positionAttribute);
        pipelineStateCreateInfo.inputAssembler.bindings.push_back(vertexBufferBinding);
        // todo primitive topology pipelineStateCreateInfo.inputAssembler
        pipelineStateCreateInfo.vertexProgram = createdVertexProgram;
        pipelineStateCreateInfo.depthStencil.depthTestEnable = true;
        pipelineStateCreateInfo.depthStencil.depthWriteEnable = true;
        pipelineStateCreateInfo.depthStencil.depthCompareFunction = CompareFunction::Less;
        pipelineStateCreateInfo.depthStencil.depthStencilFormat = options.shadowMapFormat;
        pipelineStateCreateInfo.pipelineLayout = resources->pipelineLayout;

        auto [createPipelineStateResult, createdPipelineState] = context->device->CreatePipelineState(pipelineStateCreateInfo);
        if(createPipelineStateResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->pipelineState = std::move(createdPipelineState);

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            LUCH_ASSERT(false);
            return { false };
        }

        resources->commandPool = std::move(createdCommandPool);

        BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.length = SharedUniformBufferSize;
        bufferCreateInfo.usage = BufferUsageFlags::Uniform;
        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;

        auto [createSharedBufferResult, createdSharedBuffer] = context->device->CreateBuffer(bufferCreateInfo);
        if(createSharedBufferResult != GraphicsResult::Success)
        {
            return { false };
        }

        resources->sharedBuffer = MakeUnique<SharedBuffer>(createdSharedBuffer);

        return { true, std::move(resources) };
    }

}
