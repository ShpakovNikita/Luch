#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Techniques/Forward/ForwardRendererContext.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graph;
    using namespace Graphics;

    struct EnvironmentCubemapPersistentContextCreateInfo
    {
        GraphicsDevice* device = nullptr;

        MaterialPersistentResources* materialResources = nullptr;
        CameraPersistentResources* cameraResources = nullptr;
        LightPersistentResources* lightResources = nullptr;
        IndirectLightingPersistentResources* indirectLightingResources = nullptr;
    };

    struct EnvironmentCubemapPersistentContext
    {
        GraphicsDevice* device = nullptr;

        UniquePtr<Techniques::Forward::ForwardRendererPersistentContext> rendererPersistentContext;

        RefPtr<RenderPass> renderPass;
    };

    struct EnvironmentCubemapTransientContextCreateInfo
    {
        SceneV1::Scene* scene = nullptr;
        SharedPtr<SharedBuffer> sharedBuffer;
        Vec3 position = Vec3{ 0 };
        float32 zNear = 0;
        float32 zFar = 0;
        RefPtr<DescriptorPool> descriptorPool;
    };

    struct EnvironmentCubemapTransientContext
    {
        UniquePtr<Techniques::Forward::ForwardRendererTransientContext> rendererTransientContext;

        SceneV1::Scene* scene = nullptr;
        SharedPtr<SharedBuffer> sharedBuffer;
        Vec3 position = Vec3{ 0 };
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<SceneV1::PerspectiveCamera> camera;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;
    };
}
