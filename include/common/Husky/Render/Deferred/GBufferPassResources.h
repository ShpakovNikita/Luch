#pragma once

#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct OffscreenTextures
    {
        RefPtr<Texture> baseColorTexture;
        RefPtr<Texture> normalMapTexture;
        RefPtr<Texture> depthStencilBuffer;

        RefPtr<Sampler> baseColorSampler;
        RefPtr<Sampler> normalMapSampler;
        RefPtr<Sampler> depthStencilSampler;
    };

    struct GBufferPassResources
    {
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        // Camera bindings
        DescriptorSetBinding cameraUniformBufferBinding;

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
        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialBufferDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialSamplerDescriptorSetLayout;

        Vector<OffscreenTextures> offscreen;
    };
}
