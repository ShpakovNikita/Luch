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

        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        DescriptorSetBinding cameraUniformBufferBinding;
        DescriptorSetBinding meshUniformBufferBinding;

        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;
    };

    struct GBufferTransientContext
    {
        Size2i attachmentSize;
        SceneV1::Scene* scene = nullptr;
        RefPtr<DescriptorPool> descriptorPool;
        DescriptorSet* cameraBufferDescriptorSet = nullptr;
    };
}
