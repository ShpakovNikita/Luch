#pragma once

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

    struct ResolvePersistentRenderContext
    {
        GraphicsDevice* device = nullptr;

        RefPtr<PipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding cameraUniformBufferBinding;

        Array<DescriptorSetBinding, DeferredConstants::GBufferColorAttachmentCount> colorTextureBindings;
        DescriptorSetBinding depthStencilTextureBinding;

        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> gbufferTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> gbufferSamplerDescriptorSetLayout;

        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;

        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;

        RefPtr<RenderPass> renderPass;
    };

    struct ResolveTransientRenderContext
    {
        SceneV1::Scene* scene = nullptr;
        GBufferReadOnly gbuffer;
        Size2i attachmentSize;
        SharedBuffer* sharedBuffer = nullptr;
        DescriptorSet* cameraBufferDescriptorSet = nullptr;
    };
}
