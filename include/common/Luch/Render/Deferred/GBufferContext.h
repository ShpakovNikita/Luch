#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    struct GBufferPersistentContext
    {
        GraphicsDevice* device = nullptr;

        MaterialResources* materialResources = nullptr;
        CameraResources* cameraResources = nullptr;

        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        DescriptorSetBinding meshUniformBufferBinding;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;
    };

    struct GBufferTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        Size2i attachmentSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
    };
}