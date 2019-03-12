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

    struct ResolveComputePersistentContext
    {
        GraphicsDevice* device = nullptr;

        CameraResources* cameraResources = nullptr;
        IndirectLightingResources* indirectLightingResources = nullptr;

        RefPtr<ComputePipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;

        Array<DescriptorSetBinding, DeferredConstants::GBufferColorAttachmentCount> colorTextureBindings;
        DescriptorSetBinding depthStencilTextureBinding;
        DescriptorSetBinding luminanceTextureBinding;

        RefPtr<DescriptorSetLayout> gbufferTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> luminanceTextureDescriptorSetLayout;

        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;

        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;

        RefPtr<ShaderProgram> kernelShader;
    };

    struct ResolveComputeTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        RefPtr<DescriptorPool> descriptorPool;
        SharedPtr<SharedBuffer> sharedBuffer;
        GBufferReadOnly gbuffer;
        Size2i outputSize;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> luminanceTextureDescriptorSet;
        RefPtr<DescriptorSet> gbufferTextureDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;

        RenderResource diffuseIrradianceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;

        RefPtr<DescriptorSet> indirectLightingTexturesDescriptorSet;
    };
}
