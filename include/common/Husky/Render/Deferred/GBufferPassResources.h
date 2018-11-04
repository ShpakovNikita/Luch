#pragma once

#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Render/RenderForwards.h>
#include <Husky/Render/Deferred/DeferredForwards.h>

namespace Husky::Render::Deferred
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

        ColorAttachment baseColorAttachmentTemplate;
        ColorAttachment normalMapAttachmentTemplate;
        DepthStencilAttachment depthStencilAttachmentTemplate;

        RefPtr<DescriptorSetLayout> meshBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialSamplerDescriptorSetLayout;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}
