#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::TiledDeferred
{
    using namespace Graphics;

    struct TiledDeferredPersistentContext
    {
        GraphicsDevice* device = nullptr;

        MaterialPersistentResources* materialResources = nullptr;
        CameraPersistentResources* cameraResources = nullptr;
        IndirectLightingPersistentResources* indirectLightingResources = nullptr;
        LightPersistentResources* lightResources = nullptr;

        RefPtr<DescriptorPool> descriptorPool;

        RefPtr<PipelineLayout> gbufferPipelineLayout;

        RefPtr<TiledPipelineState> resolvePipelineState;
        RefPtr<PipelineLayout> resolvePipelineLayout;

        DescriptorSetBinding meshUniformBufferBinding;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;
    };

    struct TiledDeferredTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        Size2i outputSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;

        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;

        RefPtr<DescriptorSet> indirectLightingTexturesDescriptorSet;
    };
}
