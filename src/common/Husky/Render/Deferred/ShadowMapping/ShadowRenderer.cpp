#include <Husky/Render/Deferred/ShadowMapping/ShadowRenderer.h>

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
#include <Husky/Graphics/Swapchain.h>
#include <Husky/Graphics/SwapchainInfo.h>
#include <Husky/Graphics/PipelineState.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/RenderPassCreateInfo.h>
#include <Husky/Graphics/DescriptorPoolCreateInfo.h>
#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Husky/Graphics/PipelineLayoutCreateInfo.h>
#include <Husky/Graphics/IndexType.h>
#include <Husky/Graphics/PipelineStateCreateInfo.h>

#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Light.h>

#include <Husky/Render/RenderContext.h>
#include <Husky/Render/RenderUtils.h>
#include <Husky/Render/SharedBuffer.h>

namespace Husky::Render::Deferred::ShadowMapping
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
        HUSKY_ASSERT(createCommandListResult == GraphicsResult::Success);

        shadowMaps.clear();
        commandList->Begin();

        for(const auto& lightNode : lightNodes)
        {
            const auto& light = lightNode->GetLight();
            HUSKY_ASSERT(light != nullptr);

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
            HUSKY_ASSERT_MSG(false, "Not implemented");

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

        DepthStencilAttachment attachment = resources->depthStencilAttachmentTemplate;
        attachment.output.texture = createdDepthBuffer;

        RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo
            .WithDepthStencilAttachment(&attachment);

        commandList->BeginRenderPass(renderPassCreateInfo);
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
            if (material->alphaMode != SceneV1::AlphaMode::Blend)
            {
                DrawPrimitive(primitive, commandList);
            }
        }

        // TODO sort by distance to camera
        for (const auto& primitive : mesh->GetPrimitives())
        {
            const auto& material = primitive->GetMaterial();
            if (material->alphaMode == SceneV1::AlphaMode::Blend)
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

        HUSKY_ASSERT(primitive->GetIndexBuffer().has_value());
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
        HUSKY_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);

        camera->SetDescriptorSet(RendererName, vertexDescriptorSet);
    }

    void ShadowRenderer::PrepareMesh(SceneV1::Mesh* mesh)
    {
        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
            resources->meshBufferSetLayout);

        HUSKY_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);

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
    {
        HUSKY_ASSERT(light->GetDirection().has_value());
        auto lookAt = glm::lookAt(Vec3{ 0, 0, 0 }, *light->GetDirection(), Vec3 { 0, 1, 0 });

        HUSKY_ASSERT(light->GetSpotlightAngle().has_value());
        HUSKY_ASSERT(light->GetRange().has_value());

        camera->SetYFOV(light->GetSpotlightAngle().value());
        camera->SetAspectRatio(1);
        camera->SetZNear(0.1);
        camera->SetZFar(light->GetRange().value());

        UpdateCamera(camera, transform * lookAt);
    }

    void ShadowRenderer::UpdateDirectionalLightCamera(
        SceneV1::Light* light,
        Mat4x4 transform,
        SceneV1::OrthographicCamera* camera)
    {
        HUSKY_ASSERT_MSG(false, "Not implemented");

        UpdateCamera(camera, transform);
    }

    void ShadowRenderer::UpdatePointLightCamera(
        SceneV1::Light* light,
        Mat4x4 transform,
        SceneV1::PerspectiveCamera* camera,
        int32 index)
    {
        // TODO

        HUSKY_ASSERT(light->GetRange().has_value());

        camera->SetYFOV(glm::pi<float32>() / 2);
        camera->SetAspectRatio(1);
        camera->SetZNear(0.1);
        camera->SetZFar(light->GetRange().value());

        UpdateCamera(camera, transform);
    }

    void ShadowRenderer::UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform)
    {
        camera->SetCameraViewMatrix(glm::inverse(transform));
        auto cameraUniform = RenderUtils::GetCameraUniform(camera);
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
            HUSKY_ASSERT(false);
            return { false };
        }

        resources->descriptorPool = std::move(createdDescriptorPool);

        resources->cameraUniformBufferBinding.OfType(ResourceType::UniformBuffer);
        resources->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);

        resources->depthStencilAttachmentTemplate.format = options.shadowMapFormat;
        resources->depthStencilAttachmentTemplate.depthClearValue = 1.0f;
        resources->depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
        resources->depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::Store;

        DescriptorSetLayoutCreateInfo cameraBufferSetLayoutCreateInfo;
        cameraBufferSetLayoutCreateInfo
            .OfType(DescriptorSetType::Buffer)
            .WithNBindings(1)
            .AddBinding(&resources->cameraUniformBufferBinding);

        auto [cameraBufferSetLayoutCreateResult, createdCameraBufferSetLayout] = context->device->CreateDescriptorSetLayout(
            cameraBufferSetLayoutCreateInfo);
        if(cameraBufferSetLayoutCreateResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
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
            HUSKY_ASSERT(false);
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
            HUSKY_ASSERT(false);
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
            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\todo.vert",
#endif
#if __APPLE__
    #if HUSKY_USE_METAL
            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/shadowmap_vp.metal",
    #elif HUSKY_USE_VULKAN
            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/todo.vert",
    #else
            "",
    #endif
#endif
            {});

        if(!vertexShaderLibraryCreated)
        {
            HUSKY_ASSERT(false);
            return { false };
        }

        auto [createVertexProgramResult, createdVertexProgram] = vertexShaderLibrary->CreateShaderProgram(
            ShaderStage::Vertex,
            "vp_main");

        if(createVertexProgramResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
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
            HUSKY_ASSERT(false);
            return { false };
        }

        resources->pipelineState = std::move(createdPipelineState);

        auto [createCommandPoolResult, createdCommandPool] = context->commandQueue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            HUSKY_ASSERT(false);
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
