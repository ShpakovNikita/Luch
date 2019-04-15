#include <Luch/Render/Passes/Forward/ForwardRenderPass.h>
#include <Luch/Render/Passes/Forward/ForwardContext.h>
#include <Luch/Render/Techniques/Forward/ForwardRenderer.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/IndirectLightingResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/LightResources.h>
#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/RenderGraphPassAttachmentConfig.h>
#include <Luch/Render/Graph/RenderGraphUtils.h>

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

namespace Luch::Render::Passes::Forward
{
    using namespace Graphics;
    using namespace Graph;

    const String ForwardRenderPass::RenderPassName{ "Forward" };
    const String ForwardRenderPass::RenderPassNameWithDepthPrepass{ "Forward_DepthPrepass" };

    ForwardRenderPass::ForwardRenderPass(
        ForwardPersistentContext* aPersistentContext,
        ForwardTransientContext* aTransientContext)
        : persistentContext(aPersistentContext)
        , transientContext(aTransientContext)
    {
        Utils::PopulateAttachmentConfig(attachmentConfig,
            transientContext->useDepthPrepass 
            ? persistentContext->renderPassWithDepthPrepass 
            : persistentContext->renderPass);

        renderer = MakeUnique<Techniques::Forward::ForwardRenderer>(
            persistentContext->rendererPersistentContext.get(),
            transientContext->rendererTransientContext.get());
    }

    void ForwardRenderPass::Initialize(RenderGraphBuilder* builder)
    {
        UniquePtr<RenderGraphNodeBuilder> node;

        if(transientContext->useDepthPrepass)
        {
            node = builder->AddGraphicsPass(RenderPassNameWithDepthPrepass, persistentContext->renderPassWithDepthPrepass, this);
        }
        else
        {
            node = builder->AddGraphicsPass(RenderPassName, persistentContext->renderPass, this);
        }

        auto attachmentHandles = Utils::PopulateAttachments(node.get(), attachmentConfig);

        luminanceTextureHandle = attachmentHandles.colorTextureHandles[0];
        depthStencilTextureHandle = attachmentHandles.depthStencilTextureHandle;

        if(transientContext->diffuseIlluminanceCubemapHandle)
        {
            diffuseIlluminanceCubemapHandle = node->ReadsTexture(transientContext->diffuseIlluminanceCubemapHandle);
        }

        if(transientContext->specularReflectionCubemapHandle && transientContext->specularBRDFTextureHandle)
        {
            specularReflectionCubemapHandle = node->ReadsTexture(transientContext->specularReflectionCubemapHandle);
            specularBRDFTextureHandle = node->ReadsTexture(transientContext->specularBRDFTextureHandle);
        }
    }

    ForwardRenderPass::~ForwardRenderPass() = default;

    void ForwardRenderPass::PrepareScene()
    {
        renderer->PrepareScene(transientContext->scene);
    }

    void ForwardRenderPass::UpdateScene()
    {
        renderer->UpdateScene(transientContext->scene);
    }

    void ForwardRenderPass::ExecuteGraphicsPass(
        RenderGraphResourceManager* manager,
        GraphicsCommandList* commandList)
    {
        Viewport viewport;
        viewport.width = static_cast<float32>(attachmentConfig.attachmentSize.width);
        viewport.height = static_cast<float32>(attachmentConfig.attachmentSize.height);

        Rect2i scissorRect;
        scissorRect.size = attachmentConfig.attachmentSize;

        commandList->SetViewports({ viewport });
        commandList->SetScissorRects({ scissorRect });

        auto diffuseIlluminanceCubemap = manager->GetTexture(diffuseIlluminanceCubemapHandle);
        auto specularReflectionCubemap = manager->GetTexture(specularReflectionCubemapHandle);
        auto specularBRDFTexture = manager->GetTexture(specularBRDFTextureHandle);

        renderer->DrawScene(
            transientContext->scene,
            { diffuseIlluminanceCubemap, specularReflectionCubemap, specularBRDFTexture },
            commandList);
    }

    const String& ForwardRenderPass::GetRenderPassName(bool useDepthPrepass)
    {
        return useDepthPrepass ? RenderPassNameWithDepthPrepass : RenderPassName;
    }

    ResultValue<bool, UniquePtr<ForwardPersistentContext>> ForwardRenderPass::PrepareForwardPersistentContext(
        const ForwardPersistentContextCreateInfo& createInfo)
    {
        auto context = MakeUnique<ForwardPersistentContext>();
        context->device = createInfo.device;

        const auto& supportedDepthFormats = context->device->GetPhysicalDevice()->GetCapabilities().supportedDepthFormats;
        LUCH_ASSERT_MSG(!supportedDepthFormats.empty(), "No supported depth formats");
        Format depthStencilFormat = supportedDepthFormats.front();

        {
            ColorAttachment luminanceColorAttachment;
            luminanceColorAttachment.format = LuminanceFormat;
            luminanceColorAttachment.colorLoadOperation = AttachmentLoadOperation::Clear;
            luminanceColorAttachment.colorStoreOperation = AttachmentStoreOperation::Store;
            luminanceColorAttachment.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

            DepthStencilAttachment depthStencilAttachmentTemplate;
            depthStencilAttachmentTemplate.format = depthStencilFormat;
            depthStencilAttachmentTemplate.depthLoadOperation = AttachmentLoadOperation::Clear;
            depthStencilAttachmentTemplate.depthStoreOperation = AttachmentStoreOperation::DontCare;
            depthStencilAttachmentTemplate.stencilLoadOperation = AttachmentLoadOperation::Clear;
            depthStencilAttachmentTemplate.stencilStoreOperation = AttachmentStoreOperation::DontCare;

            // Render Pass without Depth Prepass
            {
                RenderPassCreateInfo renderPassCreateInfo;
                renderPassCreateInfo.name = RenderPassName;
                renderPassCreateInfo.colorAttachments[0] = luminanceColorAttachment;
                renderPassCreateInfo.depthStencilAttachment = depthStencilAttachmentTemplate;
                renderPassCreateInfo.depthStencilAttachment->depthLoadOperation = AttachmentLoadOperation::Clear;
                renderPassCreateInfo.depthStencilAttachment->depthStoreOperation = AttachmentStoreOperation::Store;
                renderPassCreateInfo.depthStencilAttachment->stencilLoadOperation = AttachmentLoadOperation::Clear;
                renderPassCreateInfo.depthStencilAttachment->stencilStoreOperation = AttachmentStoreOperation::Store;
                renderPassCreateInfo.depthStencilAttachment->depthClearValue = 1.0;
                renderPassCreateInfo.depthStencilAttachment->stencilClearValue = 0x00000000;

                auto [createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
                if(createRenderPassResult != GraphicsResult::Success)
                {
                    return { false };
                }

                context->renderPass = std::move(createdRenderPass);
            }

            // Render Pass with Depth Prepass
            {
                RenderPassCreateInfo renderPassCreateInfo;
                renderPassCreateInfo.name = RenderPassNameWithDepthPrepass;
                renderPassCreateInfo.colorAttachments[0] = luminanceColorAttachment;
                renderPassCreateInfo.depthStencilAttachment = depthStencilAttachmentTemplate;
                renderPassCreateInfo.depthStencilAttachment->depthLoadOperation = AttachmentLoadOperation::Load;
                renderPassCreateInfo.depthStencilAttachment->depthStoreOperation = AttachmentStoreOperation::DontCare;
                renderPassCreateInfo.depthStencilAttachment->stencilLoadOperation = AttachmentLoadOperation::Load;
                renderPassCreateInfo.depthStencilAttachment->stencilStoreOperation = AttachmentStoreOperation::DontCare;

                auto [createRenderPassResult, createdRenderPass] = context->device->CreateRenderPass(renderPassCreateInfo);
                if(createRenderPassResult != GraphicsResult::Success)
                {
                    return { false };
                }

                context->renderPassWithDepthPrepass = std::move(createdRenderPass);
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
            rcci.renderPassWithDepthPrepass = context->renderPassWithDepthPrepass;

            context->rendererPersistentContext = Techniques::Forward::ForwardRenderer::PrepareForwardRendererPersistentContext(rcci);
            
            if(context->rendererPersistentContext == nullptr)
            {
                return { false };
            }
        }

        return { true, std::move(context) };
    }

    ResultValue<bool, UniquePtr<ForwardTransientContext>> ForwardRenderPass::PrepareForwardTransientContext(
        ForwardPersistentContext* persistentContext,
        const ForwardTransientContextCreateInfo& createInfo)
    {
        auto context = MakeUnique<ForwardTransientContext>();
        context->scene = createInfo.scene;
        context->sharedBuffer = createInfo.sharedBuffer;
        context->descriptorPool = createInfo.descriptorPool;
        context->cameraBufferDescriptorSet = createInfo.cameraBufferDescriptorSet;
        context->useDepthPrepass = createInfo.useDepthPrepass;
        context->diffuseIlluminanceCubemapHandle = createInfo.diffuseIlluminanceCubemapHandle;
        context->specularReflectionCubemapHandle = createInfo.specularReflectionCubemapHandle;
        context->specularBRDFTextureHandle = createInfo.specularBRDFTextureHandle;

        {
            Techniques::Forward::ForwardRendererTransientContextCreateInfo rcci;
            rcci.cameraBufferDescriptorSet = createInfo.cameraBufferDescriptorSet;
            rcci.descriptorPool = createInfo.descriptorPool;
            rcci.sharedBuffer = createInfo.sharedBuffer;
            rcci.useDepthPrepass = createInfo.useDepthPrepass;

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
