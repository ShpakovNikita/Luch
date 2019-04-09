#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Techniques/Forward/ForwardForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::Forward
{
    using namespace Graph;
    using namespace Graphics;

    struct ForwardPersistentContextCreateInfo
    {
        GraphicsDevice* device = nullptr;
        CameraPersistentResources* cameraResources = nullptr;
        MaterialPersistentResources* materialResources = nullptr;
        IndirectLightingPersistentResources* indirectLightingResources = nullptr;
        LightPersistentResources* lightResources = nullptr;
    };

    struct ForwardPersistentContext
    {
        GraphicsDevice* device = nullptr;

        RefPtr<RenderPass> renderPass;
        RefPtr<RenderPass> renderPassWithDepthPrepass;

        UniquePtr<Techniques::Forward::ForwardRendererPersistentContext> rendererPersistentContext;
    };

    struct ForwardTransientContextCreateInfo
    {
        SceneV1::Scene* scene = nullptr;
        Size2i outputSize;
        bool useDepthPrepass = false;

        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;

        RenderMutableResource depthStencilTextureHandle;

        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;
    };

    struct ForwardTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        Size2i outputSize;
        bool useDepthPrepass = false;

        UniquePtr<Techniques::Forward::ForwardRendererTransientContext> rendererTransientContext;

        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;

        RenderMutableResource depthStencilTextureHandle;

        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;
    };
}
