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

        // Mesh bindings
        DescriptorSetBinding meshUniformBufferBinding;

        // Material bindings
        DescriptorSetBinding materialUniformBufferBinding;
        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding baseColorSamplerBinding;
        DescriptorSetBinding metallicRoughnessTextureBinding;
        DescriptorSetBinding metallicRoughnessSamplerBinding;
        DescriptorSetBinding normalTextureBinding;
        DescriptorSetBinding normalSamplerBinding;
        DescriptorSetBinding occlusionTextureBinding;
        DescriptorSetBinding occlusionSamplerBinding;
        DescriptorSetBinding emissiveTextureBinding;
        DescriptorSetBinding emissiveSamplerBinding;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialSamplerDescriptorSetLayout;

        RefPtr<RenderPass> renderPass;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}
