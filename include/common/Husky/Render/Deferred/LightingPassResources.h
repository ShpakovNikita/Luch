#pragma once

#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct LightingPassResources
    {
        RefPtr<PipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding lightsUniformBufferBinding;
        RefPtr<Buffer> lightsBuffer;

        DescriptorSetBinding cameraUniformBufferBinding;
        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding baseColorSamplerBinding;
        DescriptorSetBinding normalMapTextureBinding;
        DescriptorSetBinding normalMapSamplerBinding;
        DescriptorSetBinding depthStencilTextureBinding;
        DescriptorSetBinding depthStencilSamplerBinding;

        RefPtr<Sampler> baseColorSampler;
        RefPtr<Sampler> normalMapSampler;
        RefPtr<Sampler> depthBufferSampler;

        RefPtr<DescriptorSetLayout> gbufferTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> gbufferSamplerDescriptorSetLayout;

        RefPtr<DescriptorSet> lightsDescriptorSet;

        RefPtr<DescriptorSetLayout> cameraDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> lightsDescriptorSetLayout;

        ColorAttachment colorAttachmentTemplate;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;
    };
}
