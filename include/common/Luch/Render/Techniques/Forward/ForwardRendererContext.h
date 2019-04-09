#pragma once

#include <Luch/Render/RenderForwards.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>

namespace Luch::Render::Techniques::Forward
{
    using namespace Graphics;

    struct ForwardRendererPersistentContextCreateInfo
    {
        GraphicsDevice* device = nullptr;

        MaterialPersistentResources* materialResources = nullptr;
        CameraPersistentResources* cameraResources = nullptr;
        LightPersistentResources* lightResources = nullptr;
        IndirectLightingPersistentResources* indirectLightingResources = nullptr;

        RefPtr<RenderPass> renderPass;
        RefPtr<RenderPass> renderPassWithDepthPrepass;
    };

    struct ForwardRendererPersistentContext
    {
        GraphicsDevice* device = nullptr;

        MaterialPersistentResources* materialResources = nullptr;
        CameraPersistentResources* cameraResources = nullptr;
        LightPersistentResources* lightResources = nullptr;
        IndirectLightingPersistentResources* indirectLightingResources = nullptr;

        RefPtr<PipelineLayout> pipelineLayout;

        DescriptorSetBinding meshUniformBufferBinding;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;
        RefPtr<RenderPass> renderPassWithDepthPrepass;
    };

    struct ForwardRendererTransientContextCreateInfo
    {
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;

        bool useDepthPrepass = false;

        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
    };

    struct ForwardRendererTransientContext
    {
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;

        bool useDepthPrepass = false;

        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;
        RefPtr<DescriptorSet> indirectLightingTexturesDescriptorSet;
    };
}
