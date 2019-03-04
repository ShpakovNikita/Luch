#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::Forward
{
    using namespace Graph;
    using namespace Graphics;

    struct ForwardPersistentContext
    {
        GraphicsDevice* device = nullptr;

        MaterialResources* materialResources = nullptr;
        CameraResources* cameraResources = nullptr;

        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        DescriptorSetBinding meshUniformBufferBinding;

        DescriptorSetBinding diffuseIrradianceCubemapBinding;
        DescriptorSetBinding specularReflectionCubemapBinding;
        DescriptorSetBinding specularBRDFTextureBinding;

        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;

        RefPtr<DescriptorSetLayout> indirectLightingTexturesDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;
        RefPtr<RenderPass> renderPassWithDepthOnly;
    };

    struct ForwardTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        Size2i outputSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        bool useDepthPrepass = false;
        RenderMutableResource depthStencilTextureHandle;

        RefPtr<DescriptorSet> lightsBufferDescriptorSet;

        RenderResource diffuseIrradianceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;

        RefPtr<DescriptorSet> indirectLightingTexturesDescriptorSet;
    };
}
