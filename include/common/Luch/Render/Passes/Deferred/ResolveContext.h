#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/Size2.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Passes/Deferred/DeferredForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Passes/Deferred/GBuffer.h>
#include <Luch/Render/Passes/Deferred/DeferredConstants.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::Deferred
{
    using namespace Graphics;

    struct ResolvePersistentContext
    {
        GraphicsDevice* device = nullptr;

        CameraPersistentResources* cameraResources = nullptr;
        IndirectLightingPersistentResources* indirectLightingResources = nullptr;
        LightPersistentResources* lightResources = nullptr;

        RefPtr<GraphicsPipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;

        RefPtr<Buffer> fullscreenQuadBuffer;

        Array<DescriptorSetBinding, DeferredConstants::GBufferColorAttachmentCount> colorTextureBindings;
        DescriptorSetBinding depthStencilTextureBinding;

        RefPtr<DescriptorSetLayout> gbufferTextureDescriptorSetLayout;

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
        Size2i outputSize;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> gbufferTextureDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;

        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;

        RefPtr<DescriptorSet> indirectLightingTexturesDescriptorSet;
    };
}
