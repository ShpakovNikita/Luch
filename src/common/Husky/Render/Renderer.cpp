//#include <Husky/Render/Renderer.h>
//#include <Husky/SceneV1/Scene.h>
//#include <Husky/SceneV1/Node.h>
//#include <Husky/SceneV1/Mesh.h>
//#include <Husky/SceneV1/Primitive.h>
//#include <Husky/SceneV1/Camera.h>
//#include <Husky/SceneV1/PbrMaterial.h>
//#include <Husky/SceneV1/Light.h>
//#include <Husky/SceneV1/Sampler.h>
//#include <Husky/Graphics/TextureCreateInfo.h>
//#include <Husky/Graphics/Buffer.h>
//#include <Husky/Graphics/DescriptorSet.h>
//#include <Husky/Graphics/PhysicalDevice.h>
//#include <Husky/Graphics/GraphicsDevice.h>
//#include <Husky/Graphics/CommandQueue.h>
//#include <Husky/Graphics/CommandPool.h>
//#include <Husky/Graphics/GraphicsCommandList.h>
//#include <Husky/Graphics/PipelineState.h>
//#include <Husky/Graphics/DescriptorSetBinding.h>
//#include <Husky/Graphics/RenderPassCreateInfo.h>
//#include <Husky/Graphics/DescriptorPoolCreateInfo.h>
//#include <Husky/Graphics/DescriptorSetLayoutCreateInfo.h>
//#include <Husky/Graphics/PipelineLayoutCreateInfo.h>
//
//#include <Husky/Render/RenderUtils.h>
//
//namespace Husky::Render
//{
//    using namespace Graphics;
//
//    Renderer::Renderer() = default;
//
//    void Renderer::UpdateScene(SceneV1::Scene* scene)
//    {
//        Mat4x4 identity = glm::mat4(1.0f);
//        for (const auto& node : scene->GetNodes())
//        {
//            UpdateNode(node, identity);
//        }
//    }
//
//    void Renderer::PrepareCameraNode(SceneV1::Node* node)
//    {
//        const auto& camera = node->GetCamera();
//
//        auto[createVertexDescriptorSetResult, vertexDescriptorSet] = resources->descriptorPool->AllocateDescriptorSet(
//            resources->cameraBufferDescriptorSetLayout);
//        HUSKY_ASSERT(createVertexDescriptorSetResult == GraphicsResult::Success);
//
//        camera->SetDescriptorSet(RendererName, vertexDescriptorSet);
//    }
//
//    void DeferredRenderer::PrepareMeshNode(SceneV1::Node* node)
//    {
//        const auto& mesh = node->GetMesh();
//
//        if (mesh != nullptr)
//        {
//            PrepareMesh(mesh);
//        }
//
//        // TODO sort out node hierarchy
//        for (const auto& child : node->GetChildren())
//        {
//            PrepareMeshNode(child);
//        }
//    }
//
//    void DeferredRenderer::PrepareLightNode(SceneV1::Node* node)
//    {
//        HUSKY_ASSERT_MSG(node->GetChildren().empty(), "Don't add children to light nodes");
//    }
//
//    void DeferredRenderer::PrepareNode(SceneV1::Node* node)
//    {
//    }
//
//    void DeferredRenderer::PreparePrimitive(SceneV1::Primitive* primitive)
//    {
//        RefPtr<PipelineState> pipelineState = primitive->GetPipelineState(RendererName);
//        if (pipelineState == nullptr)
//        {
//            pipelineState = CreateGBufferPipelineState(primitive);
//            primitive->SetPipelineState(RendererName, pipelineState);
//        }
//    }
//
//    void DeferredRenderer::PrepareMesh(SceneV1::Mesh* mesh)
//    {
//        for (const auto& primitive : mesh->GetPrimitives())
//        {
//            PreparePrimitive(primitive);
//        }
//
//        auto[allocateDescriptorSetResult, allocatedDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
//            gbuffer->meshBufferDescriptorSetLayout);
//
//        HUSKY_ASSERT(allocateDescriptorSetResult == GraphicsResult::Success);
//
//        mesh->SetBufferDescriptorSet(RendererName, allocatedDescriptorSet);
//    }
//
//    void DeferredRenderer::PrepareMaterial(SceneV1::PbrMaterial* material)
//    {
//        auto[allocateTextureDescriptorSetResult, textureDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
//            gbuffer->materialTextureDescriptorSetLayout);
//        HUSKY_ASSERT(allocateTextureDescriptorSetResult == GraphicsResult::Success);
//
//        auto[allocateBufferDescriptorSetResult, bufferDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
//            gbuffer->materialBufferDescriptorSetLayout);
//        HUSKY_ASSERT(allocateBufferDescriptorSetResult == GraphicsResult::Success);
//
//        auto[allocateSamplerDescriptorSetResult, samplerDescriptorSet] = gbuffer->descriptorPool->AllocateDescriptorSet(
//            gbuffer->materialSamplerDescriptorSetLayout);
//        HUSKY_ASSERT(allocateSamplerDescriptorSetResult == GraphicsResult::Success);
//
//        material->SetTextureDescriptorSet(RendererName, textureDescriptorSet);
//        material->SetBufferDescriptorSet(RendererName, bufferDescriptorSet);
//        material->SetSamplerDescriptorSet(RendererName, samplerDescriptorSet);
//
//        UpdateMaterial(material);
//    }
//
//    void DeferredRenderer::PrepareLights(SceneV1::Scene* scene)
//    {
//        constexpr int MAX_LIGHTS_COUNT = 8;
//        int32 lightsBufferSize = sizeof(LightUniform) * MAX_LIGHTS_COUNT;
//
//        BufferCreateInfo bufferCreateInfo;
//        bufferCreateInfo.length = lightsBufferSize;
//        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
//        bufferCreateInfo.usage = BufferUsageFlags::Uniform;
//
//        auto [createLightsBufferResult, createdLightsBuffer] = context->device->CreateBuffer(bufferCreateInfo);
//
//        HUSKY_ASSERT(createLightsBufferResult == GraphicsResult::Success);
//        lighting->lightsBuffer = createdLightsBuffer;
//
//        auto [mapMemoryResult, _] = lighting->lightsBuffer->MapMemory(lightsBufferSize, 0);
//        HUSKY_ASSERT(mapMemoryResult == GraphicsResult::Success);
//
//        auto [createDescriptorSetResult, createdDescriptorSet] = lighting->descriptorPool->AllocateDescriptorSet(
//            lighting->lightsBufferDescriptorSetLayout);
//        HUSKY_ASSERT(createDescriptorSetResult == GraphicsResult::Success);
//        lighting->lightsBufferDescriptorSet = createdDescriptorSet;
//
//        createdDescriptorSet->WriteUniformBuffer(lighting->lightsUniformBufferBinding, createdLightsBuffer, 0);
//        createdDescriptorSet->Update();
//    }
//
//    void DeferredRenderer::UpdateNode(SceneV1::Node* node, const Mat4x4& parentTransform)
//    {
//        const auto& mesh = node->GetMesh();
//
//        Mat4x4 localTransformMatrix;
//        const auto& localTransform = node->GetLocalTransform();
//
//        if (std::holds_alternative<Mat4x4>(localTransform))
//        {
//            localTransformMatrix = std::get<Mat4x4>(localTransform);
//        }
//        else if(std::holds_alternative<SceneV1::TransformProperties>(localTransform))
//        {
//            const auto& transformProperties = std::get<SceneV1::TransformProperties>(localTransform);
//
//            localTransformMatrix
//                = glm::translate(transformProperties.translation)
//                * glm::toMat4(transformProperties.rotation)
//                * glm::scale(transformProperties.scale);
//        }
//
//        Mat4x4 worldTransform = parentTransform * localTransformMatrix;
//        node->SetWorldTransform(worldTransform);
//
//        if (mesh != nullptr)
//        {
//            UpdateMesh(mesh, worldTransform);
//        }
//
//        const auto& camera = node->GetCamera();
//
//        if (camera != nullptr)
//        {
//            UpdateCamera(camera, worldTransform);
//        }
//
//        const auto& light = node->GetLight();
//        if (light != nullptr)
//        {
//            UpdateLight(light, worldTransform);
//        }
//
//        for (const auto& child : node->GetChildren())
//        {
//            UpdateNode(child, worldTransform);
//        }
//    }
//
//    void DeferredRenderer::UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform)
//    {
//        MeshUniform meshUniform;
//        meshUniform.transform = transform;
//        meshUniform.inverseTransform = glm::inverse(transform);
//
//        // TODO
//        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MeshUniform), 16);
//        auto descriptorSet = mesh->GetBufferDescriptorSet(RendererName);
//
//        descriptorSet->WriteUniformBuffer(
//            gbuffer->meshUniformBufferBinding,
//            suballocation.buffer,
//            suballocation.offset);
//        descriptorSet->Update();
//
//        memcpy(suballocation.offsetMemory, &meshUniform, sizeof(MeshUniform));
//    }
//
//    void DeferredRenderer::UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform)
//    {
//        camera->SetCameraViewMatrix(glm::inverse(transform));
//        auto cameraUniform = RenderUtils::GetCameraUniform(camera);
//        auto descriptorSet = camera->GetDescriptorSet(RendererName);
//
//        // TODO
//        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(CameraUniform), 16);
//
//        descriptorSet->WriteUniformBuffer(
//            resources->cameraUniformBufferBinding,
//            suballocation.buffer,
//            suballocation.offset);
//
//        descriptorSet->Update();
//
//        memcpy(suballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));
//    }
//
//    void DeferredRenderer::UpdateMaterial(SceneV1::PbrMaterial* material)
//    {
//        auto textureDescriptorSet = material->GetTextureDescriptorSet(RendererName);
//        auto samplerDescriptorSet = material->GetSamplerDescriptorSet(RendererName);
//        auto bufferDescriptorSet = material->GetBufferDescriptorSet(RendererName);
//
//        if (material->HasBaseColorTexture())
//        {
//            textureDescriptorSet->WriteTexture(
//                gbuffer->baseColorTextureBinding,
//                material->metallicRoughness.baseColorTexture.texture->GetDeviceTexture());
//
//            samplerDescriptorSet->WriteSampler(
//                gbuffer->baseColorSamplerBinding,
//                material->metallicRoughness.baseColorTexture.texture->GetDeviceSampler());
//        }
//
//        if (material->HasMetallicRoughnessTexture())
//        {
//            textureDescriptorSet->WriteTexture(
//                gbuffer->metallicRoughnessTextureBinding,
//                material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceTexture());
//
//            samplerDescriptorSet->WriteSampler(
//                gbuffer->metallicRoughnessSamplerBinding,
//                material->metallicRoughness.metallicRoughnessTexture.texture->GetDeviceSampler());
//        }
//
//        if (material->HasNormalTexture())
//        {
//            textureDescriptorSet->WriteTexture(
//                gbuffer->normalTextureBinding,
//                material->normalTexture.texture->GetDeviceTexture());
//
//            samplerDescriptorSet->WriteSampler(
//                gbuffer->normalSamplerBinding,
//                material->normalTexture.texture->GetDeviceSampler());
//        }
//
//        if (material->HasOcclusionTexture())
//        {
//            textureDescriptorSet->WriteTexture(
//                gbuffer->occlusionTextureBinding,
//                material->occlusionTexture.texture->GetDeviceTexture());
//
//            samplerDescriptorSet->WriteSampler(
//                gbuffer->occlusionSamplerBinding,
//                material->occlusionTexture.texture->GetDeviceSampler());
//        }
//
//        if (material->HasEmissiveTexture())
//        {
//            textureDescriptorSet->WriteTexture(
//                gbuffer->emissiveTextureBinding,
//                material->emissiveTexture.texture->GetDeviceTexture());
//
//            samplerDescriptorSet->WriteSampler(
//                gbuffer->emissiveSamplerBinding,
//                material->emissiveTexture.texture->GetDeviceSampler());
//        }
//
//        MaterialUniform materialUniform = RenderUtils::GetMaterialUniform(material);
//
//        // TODO
//        auto suballocation = resources->sharedBuffer->Suballocate(sizeof(MaterialUniform), 16);
//
//        bufferDescriptorSet->WriteUniformBuffer(
//            gbuffer->materialUniformBufferBinding,
//            suballocation.buffer,
//            suballocation.offset);
//
//        memcpy(suballocation.offsetMemory, &materialUniform, sizeof(MaterialUniform));
//
//        textureDescriptorSet->Update();
//        samplerDescriptorSet->Update();
//        bufferDescriptorSet->Update();
//    }
//
//    void DeferredRenderer::UpdateLight(SceneV1::Light* light, const Mat4x4& transform)
//    {
//        LightUniform lightUniform = RenderUtils::GetLightUniform(light, transform);
//
//        memcpy((LightUniform*)lighting->lightsBuffer->GetMappedMemory() + light->GetIndex(), &lightUniform, sizeof(LightUniform));
//    }
//
//    void DeferredRenderer::DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList)
//    {
//        const auto& mesh = node->GetMesh();
//        if (mesh != nullptr)
//        {
//            DrawMesh(mesh, commandList);
//        }
//
//        for (const auto& child : node->GetChildren())
//        {
//            DrawNode(child, commandList);
//        }
//    }
//
//    void DeferredRenderer::DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList)
//    {
//        commandList->BindBufferDescriptorSet(
//            ShaderStage::Vertex,
//            gbuffer->pipelineLayout,
//            mesh->GetBufferDescriptorSet(RendererName));
//
//        for (const auto& primitive : mesh->GetPrimitives())
//        {
//            const auto& material = primitive->GetMaterial();
//            if (material->alphaMode != SceneV1::AlphaMode::Blend)
//            {
//                BindMaterial(material, commandList);
//                DrawPrimitive(primitive, commandList);
//            }
//        }
//
//        // TODO sort by distance to camera
//        for (const auto& primitive : mesh->GetPrimitives())
//        {
//            const auto& material = primitive->GetMaterial();
//            if (material->alphaMode == SceneV1::AlphaMode::Blend)
//            {
//                BindMaterial(material, commandList);
//                DrawPrimitive(primitive, commandList);
//            }
//        }
//    }
//
//    void DeferredRenderer::FillGBuffer(SceneV1::Scene* scene)
//    {
//
//    }
//
//    void DeferredRenderer::CalculateLighting(SceneV1::Scene* scene)
//    {
//
//    }
//
//    void DeferredRenderer::ResolveColor()
//    {
//
//    }
//
//    void DeferredRenderer::BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList)
//    {
//        commandList->BindTextureDescriptorSet(
//            ShaderStage::Fragment,
//            gbuffer->pipelineLayout,
//            material->GetTextureDescriptorSet(RendererName));
//
//        commandList->BindBufferDescriptorSet(
//            ShaderStage::Fragment,
//            gbuffer->pipelineLayout,
//            material->GetBufferDescriptorSet(RendererName));
//
//        commandList->BindSamplerDescriptorSet(
//            ShaderStage::Fragment,
//            gbuffer->pipelineLayout,
//            material->GetSamplerDescriptorSet(RendererName));
//    }
//
//    void DeferredRenderer::DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList)
//    {
//        auto& pipelineState = primitive->GetPipelineState(RendererName);
//
//        const auto& vertexBuffers = primitive->GetVertexBuffers();
//
//        Vector<Buffer*> graphicsVertexBuffers;
//        Vector<int32> offsets;
//        graphicsVertexBuffers.reserve(vertexBuffers.size());
//        offsets.reserve(vertexBuffers.size());
//
//        for (const auto& vertexBuffer : vertexBuffers)
//        {
//            graphicsVertexBuffers.push_back(vertexBuffer.backingBuffer->GetDeviceBuffer());
//            offsets.push_back(vertexBuffer.byteOffset);
//        }
//
//        HUSKY_ASSERT(primitive->GetIndexBuffer().has_value());
//        const auto& indexBuffer = *primitive->GetIndexBuffer();
//
//        commandList->BindPipelineState(pipelineState);
//        commandList->BindVertexBuffers(graphicsVertexBuffers, offsets, 0);
//
//        commandList->BindIndexBuffer(
//                indexBuffer.backingBuffer->GetDeviceBuffer(),
//                indexBuffer.indexType,
//                indexBuffer.byteOffset);
//
//        commandList->DrawIndexedInstanced(indexBuffer.count, 0, 1, 0);
//    }
//
//    RefPtr<PipelineState> DeferredRenderer::CreateGBufferPipelineState(SceneV1::Primitive* primitive)
//    {
//        PipelineStateCreateInfo ci;
//
//        ShaderDefines<DeferredShaderDefines> shaderDefines;
//        shaderDefines.mapping = &FlagToString;
//
//        const auto& vertexBuffers = primitive->GetVertexBuffers();
//        HUSKY_ASSERT(vertexBuffers.size() == 1);
//
//        ci.inputAssembler.bindings.resize(vertexBuffers.size());
//        for (int i = 0; i < vertexBuffers.size(); i++)
//        {
//            const auto& vertexBuffer = vertexBuffers[i];
//            auto& bindingDescription = ci.inputAssembler.bindings[i];
//            bindingDescription.stride = vertexBuffer.stride;
//            bindingDescription.inputRate = VertexInputRate::PerVertex;
//        }
//
//        const auto& attributes = primitive->GetAttributes();
//        ci.inputAssembler.attributes.resize(SemanticToLocation.size());
//        for (const auto& attribute : attributes)
//        {
//            auto& attributeDescription = ci.inputAssembler.attributes[SemanticToLocation.at(attribute.semantic)];
//            attributeDescription.binding = attribute.vertexBufferIndex;
//            attributeDescription.format = attribute.format;
//            attributeDescription.offset = attribute.offset;
//
//            shaderDefines.AddFlag(SemanticToFlag.at(attribute.semantic));
//        }
//
//        // TODO
//        ci.inputAssembler.primitiveTopology = PrimitiveTopology::TriangleList;
//
//        const auto& material = primitive->GetMaterial();
//
//        if (material->doubleSided)
//        {
//            ci.rasterization.cullMode = CullMode::None;
//        }
//        else
//        {
//            ci.rasterization.cullMode = CullMode::Back;
//        }
//
//        // TODO
//        ci.rasterization.cullMode = CullMode::None;
//
//        ci.depthStencil.depthTestEnable = true;
//        ci.depthStencil.depthWriteEnable = true;
//        ci.depthStencil.depthCompareFunction = CompareFunction::Less;
//
//        auto& baseColorAttachment = ci.colorAttachments.attachments.emplace_back();
//
//        baseColorAttachment.blendEnable = material->alphaMode == SceneV1::AlphaMode::Blend;
//        baseColorAttachment.format = baseColorFormat;
//        if (baseColorAttachment.blendEnable)
//        {
//            baseColorAttachment.srcColorBlendFactor = BlendFactor::SourceAlpha;
//            baseColorAttachment.dstColorBlendFactor = BlendFactor::OneMinusSourceAlpha;
//            baseColorAttachment.colorBlendOp = BlendOperation::Add;
//            baseColorAttachment.srcAlphaBlendFactor = BlendFactor::OneMinusSourceAlpha;
//            baseColorAttachment.dstAlphaBlendFactor = BlendFactor::Zero;
//            baseColorAttachment.alphaBlendOp = BlendOperation::Add;
//        }
//
//        auto& normalMapAttachmentBlendState = ci.colorAttachments.attachments.emplace_back();
//        normalMapAttachmentBlendState.format = normalMapFormat;
//        normalMapAttachmentBlendState.blendEnable = false;
//
//        // TODO
//        //ci.renderPass = scene.gbuffer.renderPass;
//        ci.pipelineLayout = gbuffer->pipelineLayout;
//
//        if (material->HasBaseColorTexture())
//        {
//            shaderDefines.AddFlag(DeferredShaderDefines::HasBaseColorTexture);
//        }
//
//        if (material->HasMetallicRoughnessTexture())
//        {
//            shaderDefines.AddFlag(DeferredShaderDefines::HasMetallicRoughnessTexture);
//        }
//
//        if (material->HasNormalTexture())
//        {
//            shaderDefines.AddFlag(DeferredShaderDefines::HasNormalTexture);
//        }
//
//        if (material->HasOcclusionTexture())
//        {
//            shaderDefines.AddFlag(DeferredShaderDefines::HasOcclusionTexture);
//        }
//
//        if (material->HasEmissiveTexture())
//        {
//            shaderDefines.AddFlag(DeferredShaderDefines::HasEmissiveTexture);
//        }
//
//        if (material->alphaMode == SceneV1::AlphaMode::Mask)
//        {
//            shaderDefines.AddFlag(DeferredShaderDefines::AlphaMask);
//        }
//
//        auto[vertexShaderLibraryCreated, vertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
//            context->device,
//#if _WIN32
//            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.vert",
//#endif
//#if __APPLE__
//    #if HUSKY_USE_METAL
//            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/gbuffer_vp.metal",
//    #elif HUSKY_USE_VULKAN
//            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/gbuffer.vert",
//    #else
//            "",
//    #endif
//#endif
//            shaderDefines.defines);
//
//        if (!vertexShaderLibraryCreated)
//        {
//            HUSKY_ASSERT(false);
//        }
//
//        auto[vertexShaderCreateResult, createdVertexShader] = vertexShaderLibrary->CreateShaderProgram(
//            ShaderStage::Vertex,
//            "vp_main");
//        HUSKY_ASSERT(vertexShaderCreateResult == GraphicsResult::Success);
//
//        auto vertexShader = std::move(createdVertexShader);
//
//        auto[fragmentShaderLibraryCreated, fragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
//            context->device,
//#if _WIN32
//            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\gbuffer.frag",
//#endif
//#if __APPLE__
//    #if HUSKY_USE_METAL
//            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/gbuffer_fp.metal",
//    #elif HUSKY_USE_VULKAN
//            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/gbuffer.frag",
//    #else
//            "",
//    #endif
//#endif
//            shaderDefines.defines);
//
//        if (!fragmentShaderLibraryCreated)
//        {
//            HUSKY_ASSERT(false);
//        }
//
//        auto[fragmentShaderCreateResult, createdFragmentShader] = fragmentShaderLibrary->CreateShaderProgram(
//            ShaderStage::Fragment,
//            "fp_main");
//        HUSKY_ASSERT(fragmentShaderCreateResult == GraphicsResult::Success);
//
//        auto fragmentShader = std::move(createdFragmentShader);
//
//        // Retain shader modules
//        primitive->AddShaderProgram(vertexShader);
//        primitive->AddShaderProgram(fragmentShader);
//
//        ci.vertexProgram = vertexShader;
//        ci.fragmentProgram = fragmentShader;
//
//        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
//        if (createPipelineResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//        }
//
//        return createdPipeline;
//    }
//
//    RefPtr<PipelineState> DeferredRenderer::CreateLightingPipelineState(LightingPassResources* lighting)
//    {
//        PipelineStateCreateInfo ci;
//
//        auto& bindingDescription = ci.inputAssembler.bindings.emplace_back();
//        bindingDescription.stride = sizeof(QuadVertex);
//        bindingDescription.inputRate = VertexInputRate::PerVertex;
//
//        auto& positionAttributeDescription = ci.inputAssembler.attributes.emplace_back();
//        positionAttributeDescription.binding = 0;
//        positionAttributeDescription.format = Format::R32G32B32Sfloat;
//        positionAttributeDescription.offset = offsetof(QuadVertex, position);
//
//        auto& texCoordAttributeDescription = ci.inputAssembler.attributes.emplace_back();
//        texCoordAttributeDescription.binding = 0;
//        texCoordAttributeDescription.format = Format::R32G32Sfloat;
//        texCoordAttributeDescription.offset = offsetof(QuadVertex, texCoord);
//
//        ci.inputAssembler.primitiveTopology = PrimitiveTopology::TriangleList;
//        ci.rasterization.cullMode = CullMode::Back;
//        ci.rasterization.frontFace = FrontFace::CounterClockwise;
//
//        ci.depthStencil.depthTestEnable = false;
//        ci.depthStencil.depthWriteEnable = false;
//
//        auto& outputAttachment = ci.colorAttachments.attachments.emplace_back();
//        outputAttachment.format = context->swapchain->GetInfo().format;
//        outputAttachment.blendEnable = false;
//
//        ci.depthStencil.depthStencilFormat = Format::Undefined;
//
//        //pipelineState.renderPass = lighting.renderPass;
//        ci.pipelineLayout = lighting->pipelineLayout;
//        ci.vertexProgram = lighting->vertexShader;
//        ci.fragmentProgram = lighting->fragmentShader;
//
//        auto[createPipelineResult, createdPipeline] = context->device->CreatePipelineState(ci);
//        if (createPipelineResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//        }
//
//        return createdPipeline;
//    }
//
//    ResultValue<bool, UniquePtr<GBufferPassResources>> DeferredRenderer::PrepareGBufferPassResources()
//    {
//        UniquePtr<GBufferPassResources> gbufferResources = MakeUnique<GBufferPassResources>();
//
//        auto [offscreenTexturesCreated, createdOffscreenTextures] = CreateOffscreenTextures();
//        if (!offscreenTexturesCreated)
//        {
//            return { false };
//        }
//
//        gbufferResources->offscreen = std::move(createdOffscreenTextures);
//
//        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
//        descriptorPoolCreateInfo.maxDescriptorSets = MaxDescriptorSetCount;
//        descriptorPoolCreateInfo.descriptorCount =
//        {
//            { ResourceType::Texture, MaxDescriptorSetCount },
//            { ResourceType::UniformBuffer, MaxDescriptorSetCount },
//            { ResourceType::Sampler, MaxDescriptorSetCount },
//        };
//
//        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
//            descriptorPoolCreateInfo);
//
//        if (createDescriptorPoolResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        gbufferResources->descriptorPool = std::move(createdDescriptorPool);
//
//        gbufferResources->materialUniformBufferBinding.OfType(ResourceType::UniformBuffer);
//        gbufferResources->meshUniformBufferBinding.OfType(ResourceType::UniformBuffer);
//
//        gbufferResources->baseColorTextureBinding.OfType(ResourceType::Texture);
//        gbufferResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);
//
//        gbufferResources->metallicRoughnessTextureBinding.OfType(ResourceType::Texture);
//        gbufferResources->metallicRoughnessSamplerBinding.OfType(ResourceType::Sampler);
//
//        gbufferResources->normalTextureBinding.OfType(ResourceType::Texture);
//        gbufferResources->normalSamplerBinding.OfType(ResourceType::Sampler);
//
//        gbufferResources->occlusionTextureBinding.OfType(ResourceType::Texture);
//        gbufferResources->occlusionSamplerBinding.OfType(ResourceType::Sampler);
//
//        gbufferResources->emissiveTextureBinding.OfType(ResourceType::Texture);
//        gbufferResources->emissiveSamplerBinding.OfType(ResourceType::Sampler);
//
//        DescriptorSetLayoutCreateInfo meshDescriptorSetLayoutCreateInfo;
//        meshDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Buffer)
//            .AddBinding(&gbufferResources->meshUniformBufferBinding);
//
//        DescriptorSetLayoutCreateInfo materialBufferDescriptorSetLayoutCreateInfo;
//        materialBufferDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Buffer)
//            .AddBinding(&gbufferResources->materialUniformBufferBinding);
//
//        DescriptorSetLayoutCreateInfo materialTextureDescriptorSetLayoutCreateInfo;
//        materialTextureDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Texture)
//            .WithNBindings(5)
//            .AddBinding(&gbufferResources->baseColorTextureBinding)
//            .AddBinding(&gbufferResources->metallicRoughnessTextureBinding)
//            .AddBinding(&gbufferResources->normalTextureBinding)
//            .AddBinding(&gbufferResources->occlusionTextureBinding)
//            .AddBinding(&gbufferResources->emissiveTextureBinding);
//
//        DescriptorSetLayoutCreateInfo materialSamplerDescriptorSetLayoutCreateInfo;
//        materialSamplerDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Sampler)
//            .WithNBindings(5)
//            .AddBinding(&gbufferResources->baseColorSamplerBinding)
//            .AddBinding(&gbufferResources->metallicRoughnessSamplerBinding)
//            .AddBinding(&gbufferResources->normalSamplerBinding)
//            .AddBinding(&gbufferResources->occlusionSamplerBinding)
//            .AddBinding(&gbufferResources->emissiveSamplerBinding);
//
//        auto[createMeshDescriptorSetLayoutResult, createdMeshDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
//            meshDescriptorSetLayoutCreateInfo);
//
//        if (createMeshDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        gbufferResources->meshBufferDescriptorSetLayout = std::move(createdMeshDescriptorSetLayout);
//
//        auto[createMaterialTextureDescriptorSetLayoutResult, createdMaterialTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
//            materialTextureDescriptorSetLayoutCreateInfo);
//
//        if (createMaterialTextureDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        gbufferResources->materialTextureDescriptorSetLayout = std::move(createdMaterialTextureDescriptorSetLayout);
//
//        auto[createMaterialBufferDescriptorSetLayoutResult, createdMaterialBufferDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
//            materialBufferDescriptorSetLayoutCreateInfo);
//
//        if (createMaterialBufferDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        gbufferResources->materialBufferDescriptorSetLayout = std::move(createdMaterialBufferDescriptorSetLayout);
//
//        auto[createMaterialSamplerDescriptorSetLayoutResult, createdMaterialSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(
//            materialSamplerDescriptorSetLayoutCreateInfo);
//
//        if (createMaterialSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        gbufferResources->materialSamplerDescriptorSetLayout = std::move(createdMaterialSamplerDescriptorSetLayout);
//
//        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
//        pipelineLayoutCreateInfo
//            .AddSetLayout(ShaderStage::Vertex, resources->cameraBufferDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Vertex, gbufferResources->meshBufferDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialTextureDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialBufferDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Fragment, gbufferResources->materialSamplerDescriptorSetLayout);
//
//        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(
//            pipelineLayoutCreateInfo);
//
//        if (createPipelineLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        gbufferResources->pipelineLayout = std::move(createdPipelineLayout);
//
//        gbufferResources->baseColorAttachmentTemplate.format = baseColorFormat;
//        gbufferResources->baseColorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
//        gbufferResources->baseColorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
//        gbufferResources->baseColorAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//        gbufferResources->normalMapAttachmentTemplate.format = normalMapFormat;
//        gbufferResources->normalMapAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
//        gbufferResources->normalMapAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
//        gbufferResources->normalMapAttachmentTemplate.clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
//
//        gbufferResources->depthStencilAttachmentTemplate.format = depthStencilFormat;
//        gbufferResources->depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
//        gbufferResources->depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::Store;
//        gbufferResources->depthStencilAttachmentTemplate.stencilLoadOperation = AttachmentLoadOperation::Clear;
//        gbufferResources->depthStencilAttachmentTemplate.stencilStoreOperation = AttachmentStoreOperation::Store;
//        gbufferResources->depthStencilAttachmentTemplate.depthClearValue = maxDepth;
//        gbufferResources->depthStencilAttachmentTemplate.stencilClearValue = 0xffffffff;
//
//        return { true, std::move(gbufferResources) };
//    }
//
//    ResultValue<bool, UniquePtr<LightingPassResources>> DeferredRenderer::PrepareLightingPassResources(
//        GBufferPassResources* gbufferPassResources)
//    {
//        UniquePtr<LightingPassResources> lightingResources = MakeUnique<LightingPassResources>();
//        int32 swapchainLength = context->swapchain->GetInfo().imageCount;
//
//        DescriptorPoolCreateInfo descriptorPoolCreateInfo;
//        descriptorPoolCreateInfo.maxDescriptorSets = 1 + swapchainLength + 1;
//        descriptorPoolCreateInfo.descriptorCount =
//        {
//            { ResourceType::UniformBuffer, 1 },
//            { ResourceType::Texture, swapchainLength * OffscreenImageCount + 1 },
//            { ResourceType::Sampler, swapchainLength * OffscreenImageCount + 1 },
//        };
//
//        auto[createDescriptorPoolResult, createdDescriptorPool] = context->device->CreateDescriptorPool(
//            descriptorPoolCreateInfo);
//        if (createDescriptorPoolResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->descriptorPool = std::move(createdDescriptorPool);
//
//        ShaderDefines<DeferredShaderDefines> shaderDefines;
//        shaderDefines.mapping = &FlagToString;
//
//        auto [vertexShaderLibraryCreated, createdVertexShaderLibrary] = RenderUtils::CreateShaderLibrary(
//            context->device,
//#if _WIN32
//            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.vert",
//#endif
//#if __APPLE__
//    #if HUSKY_USE_METAL
//            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_vp.metal",
//    #elif HUSKY_USE_VULKAN
//            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.vert",
//    #else
//            "",
//    #endif
//#endif
//            shaderDefines.defines);
//
//        if (!vertexShaderLibraryCreated)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        auto [vertexShaderProgramCreateResult, vertexShaderProgram] = createdVertexShaderLibrary->CreateShaderProgram(ShaderStage::Vertex, "vp_main");
//        if(vertexShaderProgramCreateResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->vertexShader = std::move(vertexShaderProgram);
//
//        auto[fragmentShaderLibraryCreated, createdFragmentShaderLibrary] = RenderUtils::CreateShaderLibrary(
//            context->device,
//#if _WIN32
//            "C:\\Development\\Husky\\src\\Husky\\Render\\Shaders\\Deferred\\lighting.frag",
//#endif
//#if __APPLE__
//    #if HUSKY_USE_METAL
//            "/Users/spo1ler/Development/HuskyEngine/src/Metal/Husky/Render/Shaders/Deferred/lighting_fp.metal",
//    #elif HUSKY_USE_VULKAN
//            "/Users/spo1ler/Development/HuskyEngine/src/Vulkan/Husky/Render/Shaders/Deferred/lighting.frag",
//    #else
//            "",
//    #endif
//#endif
//            shaderDefines.defines);
//
//        if (!fragmentShaderLibraryCreated)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        auto [fragmentShaderProgramCreateResult, fragmentShaderProgram] = createdFragmentShaderLibrary->CreateShaderProgram(
//            ShaderStage::Fragment,
//            "fp_main");
//
//        if(fragmentShaderProgramCreateResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->fragmentShader = std::move(fragmentShaderProgram);
//
//        lightingResources->lightsUniformBufferBinding.OfType(ResourceType::UniformBuffer);
//
//        DescriptorSetLayoutCreateInfo lightsDescriptorSetLayoutCreateInfo;
//        lightsDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Buffer)
//            .WithNBindings(1)
//            .AddBinding(&lightingResources->lightsUniformBufferBinding);
//
//        auto[createLightsDescriptorSetLayoutResult, createdLightsDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(lightsDescriptorSetLayoutCreateInfo);
//        if (createLightsDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->lightsBufferDescriptorSetLayout = std::move(createdLightsDescriptorSetLayout);
//
//        lightingResources->baseColorTextureBinding.OfType(ResourceType::Texture);
//        lightingResources->baseColorSamplerBinding.OfType(ResourceType::Sampler);
//        lightingResources->normalMapTextureBinding.OfType(ResourceType::Texture);
//        lightingResources->normalMapSamplerBinding.OfType(ResourceType::Sampler);
//        lightingResources->depthStencilTextureBinding.OfType(ResourceType::Texture);
//        lightingResources->depthStencilSamplerBinding.OfType(ResourceType::Sampler);
//
//        DescriptorSetLayoutCreateInfo gbufferTextureDescriptorSetLayoutCreateInfo;
//        gbufferTextureDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Texture)
//            .WithNBindings(3)
//            .AddBinding(&lightingResources->baseColorTextureBinding)
//            .AddBinding(&lightingResources->normalMapTextureBinding)
//            .AddBinding(&lightingResources->depthStencilTextureBinding);
//
//        auto[createGBufferTextureDescriptorSetLayoutResult, createdGBufferTextureDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferTextureDescriptorSetLayoutCreateInfo);
//        if (createGBufferTextureDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->gbufferTextureDescriptorSetLayout = std::move(createdGBufferTextureDescriptorSetLayout);
//
//        DescriptorSetLayoutCreateInfo gbufferSamplerDescriptorSetLayoutCreateInfo;
//        gbufferSamplerDescriptorSetLayoutCreateInfo
//            .OfType(DescriptorSetType::Sampler)
//            .WithNBindings(3)
//            .AddBinding(&lightingResources->baseColorSamplerBinding)
//            .AddBinding(&lightingResources->normalMapSamplerBinding)
//            .AddBinding(&lightingResources->depthStencilSamplerBinding);
//
//        auto[createGBufferSamplerDescriptorSetLayoutResult, createdGBufferSamplerDescriptorSetLayout] = context->device->CreateDescriptorSetLayout(gbufferSamplerDescriptorSetLayoutCreateInfo);
//        if (createGBufferSamplerDescriptorSetLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->gbufferSamplerDescriptorSetLayout = std::move(createdGBufferSamplerDescriptorSetLayout);
//
//        SamplerCreateInfo samplerCreateInfo;
//
//        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(samplerCreateInfo);
//        if (createBaseColorSamplerResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->baseColorSampler = std::move(createdBaseColorSampler);
//
//        auto[createNormalMapSamplerResult, createdNormalMapSampler] = context->device->CreateSampler(samplerCreateInfo);
//        if (createNormalMapSamplerResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->normalMapSampler = std::move(createdNormalMapSampler);
//
//        auto[createDepthSamplerResult, createdDepthSampler] = context->device->CreateSampler(samplerCreateInfo);
//        if (createDepthSamplerResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->depthBufferSampler = std::move(createdDepthSampler);
//
//        PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
//        pipelineLayoutCreateInfo
//            .AddSetLayout(ShaderStage::Fragment, resources->cameraBufferDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Fragment, lightingResources->lightsBufferDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Fragment, lightingResources->gbufferTextureDescriptorSetLayout)
//            .AddSetLayout(ShaderStage::Fragment, lightingResources->gbufferSamplerDescriptorSetLayout);
//
//        auto[createPipelineLayoutResult, createdPipelineLayout] = context->device->CreatePipelineLayout(pipelineLayoutCreateInfo);
//        if (createPipelineLayoutResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->pipelineLayout = std::move(createdPipelineLayout);
//
//        lightingResources->colorAttachmentTemplate.format = context->swapchain->GetInfo().format;
//        lightingResources->colorAttachmentTemplate.colorLoadOperation = AttachmentLoadOperation::Clear;
//        lightingResources->colorAttachmentTemplate.colorStoreOperation = AttachmentStoreOperation::Store;
//
//        // TODO result
//        lightingResources->pipelineState = CreateLightingPipelineState(lightingResources.get());
//
//        BufferCreateInfo bufferCreateInfo;
//        bufferCreateInfo.length = fullscreenQuadVertices.size() * sizeof(QuadVertex);
//        bufferCreateInfo.storageMode = ResourceStorageMode::Shared;
//        bufferCreateInfo.usage = BufferUsageFlags::VertexBuffer;
//
//        auto[createQuadBufferResult, createdQuadBuffer] = context->device->CreateBuffer(
//            bufferCreateInfo,
//            fullscreenQuadVertices.data());
//
//        if (createQuadBufferResult != GraphicsResult::Success)
//        {
//            HUSKY_ASSERT(false);
//            return { false };
//        }
//
//        lightingResources->fullscreenQuadBuffer = std::move(createdQuadBuffer);
//
//        auto [allocateTextureSetResult, allocatedTextureSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
//                lightingResources->gbufferTextureDescriptorSetLayout);
//
//        if (allocateTextureSetResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        lightingResources->gbufferTextureDescriptorSet = std::move(allocatedTextureSet);
//
//        auto [allocateSamplerSetResult, allocatedSamplerSet] = lightingResources->descriptorPool->AllocateDescriptorSet(
//            lightingResources->gbufferSamplerDescriptorSetLayout);
//
//        if (allocateSamplerSetResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        lightingResources->gbufferSamplerDescriptorSet = std::move(allocatedSamplerSet);
//
//        lightingResources->gbufferTextureDescriptorSet->WriteTexture(
//            lightingResources->baseColorTextureBinding,
//            gbufferPassResources->offscreen.baseColorTexture);
//
//        lightingResources->gbufferTextureDescriptorSet->WriteTexture(
//            lightingResources->normalMapTextureBinding,
//            gbufferPassResources->offscreen.normalMapTexture);
//
//        lightingResources->gbufferTextureDescriptorSet->WriteTexture(
//            lightingResources->depthStencilTextureBinding,
//            gbufferPassResources->offscreen.depthStencilBuffer);
//
//        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
//            lightingResources->baseColorSamplerBinding,
//            gbufferPassResources->offscreen.baseColorSampler);
//
//        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
//            lightingResources->normalMapSamplerBinding,
//            gbufferPassResources->offscreen.normalMapSampler);
//
//        lightingResources->gbufferSamplerDescriptorSet->WriteSampler(
//            lightingResources->depthStencilSamplerBinding,
//            gbufferPassResources->offscreen.depthStencilSampler);
//
//        return { true, std::move(lightingResources) };
//    }
//
//    ResultValue<bool, OffscreenTextures> DeferredRenderer::CreateOffscreenTextures()
//    {
//        OffscreenTextures textures;
//
//        const auto& swapchainCreateInfo = context->swapchain->GetInfo();
//        int32 textureWidth = swapchainCreateInfo.width;
//        int32 textureHeight = swapchainCreateInfo.height;
//
//        TextureCreateInfo baseColorTextureCreateInfo;
//        baseColorTextureCreateInfo.format = baseColorFormat;
//        baseColorTextureCreateInfo.width = textureWidth;
//        baseColorTextureCreateInfo.height = textureHeight;
//        baseColorTextureCreateInfo.usage =
//              TextureUsageFlags::ColorAttachment
//            | TextureUsageFlags::ShaderRead
//            | TextureUsageFlags::TransferSource;
//
//        SamplerCreateInfo baseColorSamplerCreateInfo;
//
//        auto[createBaseColorTextureResult, createdBaseColorTexture] = context->device->CreateTexture(baseColorTextureCreateInfo);
//        if (createBaseColorTextureResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        auto[createBaseColorSamplerResult, createdBaseColorSampler] = context->device->CreateSampler(baseColorSamplerCreateInfo);
//        if (createBaseColorSamplerResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        textures.baseColorTexture = std::move(createdBaseColorTexture);
//        textures.baseColorSampler = std::move(createdBaseColorSampler);
//
//        // Normal map
//
//        TextureCreateInfo normalMapCreateInfo;
//        normalMapCreateInfo.format = normalMapFormat;
//        normalMapCreateInfo.width = textureWidth;
//        normalMapCreateInfo.height = textureHeight;
//        normalMapCreateInfo.usage =
//              TextureUsageFlags::ColorAttachment
//            | TextureUsageFlags::ShaderRead
//            | TextureUsageFlags::TransferSource;
//
//        SamplerCreateInfo normalMapSamplerCreateInfo;
//
//        auto[createNormapMapTextureResult, createdNormapMapTexture] = context->device->CreateTexture(normalMapCreateInfo);
//        if (createNormapMapTextureResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        auto[createNormapMapSamplerResult, createdNormapMapSampler] = context->device->CreateSampler(normalMapSamplerCreateInfo);
//        if (createNormapMapTextureResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        textures.normalMapTexture = std::move(createdNormapMapTexture);
//        textures.normalMapSampler = std::move(createdNormapMapSampler);
//
//        TextureCreateInfo depthBufferCreateInfo;
//        depthBufferCreateInfo.format = depthStencilFormat;
//        depthBufferCreateInfo.width = textureWidth;
//        depthBufferCreateInfo.height = textureHeight;
//        depthBufferCreateInfo.usage =
//              TextureUsageFlags::DepthStencilAttachment
//            | TextureUsageFlags::ShaderRead
//            | TextureUsageFlags::TransferSource;
//
//        SamplerCreateInfo depthBufferSamplerCreateInfo;
//
//        auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = context->device->CreateTexture(depthBufferCreateInfo);
//        if (createDepthStencilBufferResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        auto[createDepthStencilSamplerResult, createdDepthStencilSampler] = context->device->CreateSampler(depthBufferSamplerCreateInfo);
//        if (createDepthStencilSamplerResult != GraphicsResult::Success)
//        {
//            return { false };
//        }
//
//        textures.depthStencilBuffer = std::move(createdDepthStencilBuffer);
//        textures.depthStencilSampler = std::move(createdDepthStencilSampler);
//
//        return { true, textures };
//    }
//}
//
