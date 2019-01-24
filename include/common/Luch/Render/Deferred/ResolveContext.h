#pragma once

#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Deferred/GBuffer.h>
#include <Luch/Render/Deferred/DeferredConstants.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/RefPtr.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    struct ResolvePersistentContext
    {
        GraphicsDevice* device = nullptr;

        CameraResources* cameraResources = nullptr;

        RefPtr<GraphicsPipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;

        RefPtr<Buffer> fullscreenQuadBuffer;

        Array<DescriptorSetBinding, DeferredConstants::GBufferColorAttachmentCount> colorTextureBindings;
        DescriptorSetBinding depthStencilTextureBinding;

        RefPtr<DescriptorSetLayout> gbufferTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> gbufferSamplerDescriptorSetLayout;

        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;

        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;

        RefPtr<RenderPass> renderPass;
    };

    struct ResolveTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        RefPtr<DescriptorPool> descriptorPool;
        SharedPtr<SharedBuffer> sharedBuffer;
        GBufferReadOnly gbuffer;
        Size2i attachmentSize;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> gbufferTextureDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;
    };
}
