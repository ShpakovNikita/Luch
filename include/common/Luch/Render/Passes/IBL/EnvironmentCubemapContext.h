#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graph;
    using namespace Graphics;

    struct EnvironmentCubemapPersistentContext
    {
        GraphicsDevice* device = nullptr;

        MaterialResources* materialResources = nullptr;
        CameraResources* cameraResources = nullptr;

        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        DescriptorSetBinding meshUniformBufferBinding;

        DescriptorSetBinding cubemapBufferBinding;

        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;

        RefPtr<SceneV1::PerspectiveCamera> camera;

        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> cubemapBufferDescriptorSetLayout;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;
    };

    struct EnvironmentCubemapTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        Size2i outputSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        Vec3 position = Vec3{ 0 };
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;
        RefPtr<DescriptorSet> cubemapBufferDescriptorSet;
    };
}
