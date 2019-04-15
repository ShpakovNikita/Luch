#include <Luch/Render/Passes/IBL/EnvironmentCubemapRenderPass.h>
#include <Luch/Render/Passes/IBL/EnvironmentCubemapContext.h>
#include <Luch/Render/Passes/IBL/IBLCommon.h>
#include <Luch/Render/CubemapCommon.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/LightResources.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/RenderGraphUtils.h>
#include <Luch/Render/Techniques/Forward/ForwardRenderer.h>
#include <Luch/Render/Techniques/Forward/ForwardRendererContext.h>

#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/AlphaMode.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/AttributeSemantic.h>

#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/Graphics/BufferCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/ShaderLibrary.h>
#include <Luch/Graphics/DescriptorSet.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/PhysicalDeviceCapabilities.h>
#include <Luch/Graphics/GraphicsPipelineState.h>
#include <Luch/Graphics/GraphicsPipelineStateCreateInfo.h>
#include <Luch/Graphics/TiledPipelineStateCreateInfo.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Graphics/DescriptorSetLayoutCreateInfo.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Graphics/IndexType.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;
    using namespace Graph;

    const String EnvironmentCubemapRenderPass::RenderPassName{ "EnvironmentCubemap" };

    EnvironmentCubemapRenderPass::EnvironmentCubemapRenderPass(
        EnvironmentCubemapPersistentContext* aPersistentContext,
        EnvironmentCubemapTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        Utils::PopulateAttachmentConfig(attachmentConfig, persistentContext->renderPass);

        renderer = MakeUnique<Techniques::Forward::ForwardRenderer>(
            persistentContext->rendererPersistentContext.get(),
            transientContext->rendererTransientContext.get());
    }

    EnvironmentCubemapRenderPass::~EnvironmentCubemapRenderPass() = default;

    void EnvironmentCubemapRenderPass::PrepareScene()
    {
        renderer->PrepareScene(transientContext->scene);
    }

    void EnvironmentCubemapRenderPass::UpdateScene()
    {
        renderer->UpdateScene(transientContext->scene);
    }

    void EnvironmentCubemapRenderPass::Initialize(RenderGraphBuilder* builder)
    {
        // Layer selection is funky on macOS and on iOS it's only supported on A12, so
        // just render to separate faces in separate passes
        // TODO support layer selection (will allow to render cubemap in a single pass)

        face = attachmentConfig.colorAttachments[0]->descriptor.slice;

        auto node = builder->AddGraphicsPass(RenderPassName, persistentContext->renderPass, this);

        auto attachmentHandles = Utils::PopulateAttachments(node.get(), attachmentConfig);

        luminanceCubemapHandle = attachmentHandles.colorTextureHandles[0];
        depthCubemapHandle = attachmentHandles.depthStencilTextureHandle;
    }

    void EnvironmentCubemapRenderPass::ExecuteGraphicsPass(
        RenderGraphResourceManager* manager [[ maybe_unused ]],
        GraphicsCommandList* commandList)
    {
        Mat4x4 cameraTransform
            = glm::translate(transientContext->position)
            * glm::toMat4(glm::quatLookAt(CubemapCommon::CubemapNormal[face], CubemapCommon::CubemapUp[face]));

        CameraUniform cameraUniform;
        cameraUniform = RenderUtils::GetCameraUniform(transientContext->camera, cameraTransform);

        auto cameraSuballocation = transientContext->sharedBuffer->Suballocate(sizeof(CameraUniform), 256);
        memcpy(cameraSuballocation.offsetMemory, &cameraUniform, sizeof(CameraUniform));

        transientContext->cameraBufferDescriptorSet->WriteUniformBuffer(
            persistentContext->rendererPersistentContext->cameraResources->cameraUniformBufferBinding,
            cameraSuballocation.buffer,
            cameraSuballocation.offset);

        transientContext->cameraBufferDescriptorSet->Update();

        Viewport viewport;
        viewport.width = static_cast<float32>(attachmentConfig.attachmentSize.width);
        viewport.height = static_cast<float32>(attachmentConfig.attachmentSize.height);

        Rect2i scissorRect;
        scissorRect.size = attachmentConfig.attachmentSize;

        commandList->SetViewports({ viewport });
        commandList->SetScissorRects({ scissorRect });

        renderer->DrawScene(transientContext->scene, {}, commandList);
    }

    ResultValue<bool, UniquePtr<EnvironmentCubemapPersistentContext>> EnvironmentCubemapRenderPass::PrepareEnvironmentCubemapPersistentContext(
        const EnvironmentCubemapPersistentContextCreateInfo& createInfo)
    {
        auto context = MakeUnique<EnvironmentCubemapPersistentContext>();
        context->device = createInfo.device;

        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        {
            {
                ColorAttachment luminanceColorAttachment;
                luminanceColorAttachment.format = LuminanceFormat;
                luminanceColorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
                luminanceColorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;
                luminanceColorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

                DepthStencilAttachment depthStencilAttachment;
                depthStencilAttachment.format = depthStencilFormat;
                depthStencilAttachment.depthLoadOperation = AttachmentLoadOperation::Clear;
                depthStencilAttachment.depthStoreOperation = AttachmentStoreOperation::DontCare;
                depthStencilAttachment.stencilLoadOperation = AttachmentLoadOperation::Clear;
                depthStencilAttachment.stencilStoreOperation = AttachmentStoreOperation::DontCare;
                
                RenderPassCreateInfo renderPassCreateInfo;
                renderPassCreateInfo.name = RenderPassName;
                renderPassCreateInfo.colorAttachments[0] = luminanceColorAttachment;
                renderPassCreateInfo.depthStencilAttachment = depthStencilAttachment;

                auto [createRenderPassResult, createdRenderPass] = createInfo.device->CreateRenderPass(renderPassCreateInfo);
                if(createRenderPassResult != GraphicsResult::Success)
                {
                    return { false };
                }

                context->renderPass = std::move(createdRenderPass);
            }
        }

        {
            Techniques::Forward::ForwardRendererPersistentContextCreateInfo rcci;
            rcci.device = createInfo.device;
            rcci.cameraResources = createInfo.cameraResources;
            rcci.lightResources = createInfo.lightResources;
            rcci.materialResources = createInfo.materialResources;
            rcci.indirectLightingResources = createInfo.indirectLightingResources;
            rcci.renderPass = context->renderPass;

            context->rendererPersistentContext = Techniques::Forward::ForwardRenderer::PrepareForwardRendererPersistentContext(rcci);
            if(context->rendererPersistentContext == nullptr)
            {
                return { false };
            }
        }

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<EnvironmentCubemapTransientContext>> EnvironmentCubemapRenderPass::PrepareEnvironmentCubemapTransientContext(
        EnvironmentCubemapPersistentContext* persistentContext,
        const EnvironmentCubemapTransientContextCreateInfo& createInfo)
    {
        auto context = MakeUnique<EnvironmentCubemapTransientContext>();
        context->descriptorPool = createInfo.descriptorPool;
        context->scene = createInfo.scene;
        context->position = createInfo.position;
        context->sharedBuffer = createInfo.sharedBuffer;

        context->camera = MakeRef<SceneV1::PerspectiveCamera>(glm::pi<float32>()/2, createInfo.zNear, createInfo.zFar, 1);

        {
            auto [result, descriptorSet] = createInfo.descriptorPool->AllocateDescriptorSet(
                persistentContext->rendererPersistentContext->cameraResources->cameraBufferDescriptorSetLayout);
            
            if(result != GraphicsResult::Success)
            {
                return { false };
            }

            context->cameraBufferDescriptorSet = std::move(descriptorSet);
        }

        {
            Techniques::Forward::ForwardRendererTransientContextCreateInfo rcci;
            rcci.cameraBufferDescriptorSet = context->cameraBufferDescriptorSet;
            rcci.descriptorPool = context->descriptorPool;
            rcci.sharedBuffer = context->sharedBuffer;
            rcci.useDepthPrepass = false;

            context->rendererTransientContext = Techniques::Forward::ForwardRenderer::PrepareForwardRendererTransientContext(
                persistentContext->rendererPersistentContext.get(),
                rcci);
            
            if(context->rendererTransientContext == nullptr)
            {
                return { false };
            }
        }

        return { true, std::move(context) };
    }
}
