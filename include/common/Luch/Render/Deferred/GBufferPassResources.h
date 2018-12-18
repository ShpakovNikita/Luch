#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    struct GBufferPassResources
    {
        RefPtr<CommandPool> commandPool;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        // Camera bindings
        DescriptorSetBinding cameraUniformBufferBinding;

        // Mesh bindings
        DescriptorSetBinding meshUniformBufferBinding;

        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}
