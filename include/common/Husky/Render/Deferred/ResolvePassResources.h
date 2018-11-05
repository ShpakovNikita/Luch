#pragma once

#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct ResolvePassResources
    {
        RefPtr<PipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<CommandPool> commandPool;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding baseColorSamplerBinding;
        DescriptorSetBinding normalMapTextureBinding;
        DescriptorSetBinding normalMapSamplerBinding;
        DescriptorSetBinding depthStencilTextureBinding;
        DescriptorSetBinding depthStencilSamplerBinding;

        RefPtr<Sampler> baseColorSampler;
        RefPtr<Sampler> normalMapSampler;
        RefPtr<Sampler> depthStencilSampler;

        RefPtr<DescriptorSet> gbufferTextureDescriptorSet;
        RefPtr<DescriptorSet> gbufferSamplerDescriptorSet;

        RefPtr<DescriptorSetLayout> gbufferTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> gbufferSamplerDescriptorSetLayout;

        DescriptorSetBinding lightingParamsBinding;
        DescriptorSetBinding lightsBufferBinding;
        RefPtr<Buffer> lightsBuffer;

        RefPtr<DescriptorSetLayout> lightsBufferDescriptorSetLayout;
        RefPtr<DescriptorSet> lightsBufferDescriptorSet;

        ColorAttachment colorAttachmentTemplate;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}
