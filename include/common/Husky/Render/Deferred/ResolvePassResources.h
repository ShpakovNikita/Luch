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
        DescriptorSetBinding diffuseTextureBinding;
        DescriptorSetBinding diffuseSamplerBinding;
        DescriptorSetBinding specularTextureBinding;
        DescriptorSetBinding specularSamplerBinding;

        RefPtr<Sampler> sampler;

        RefPtr<DescriptorSetLayout> textureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> samplerDescriptorSetLayout;

        RefPtr<DescriptorSet> textureDescriptorSet;
        RefPtr<DescriptorSet> samplerDescriptorSet;

        ColorAttachment colorAttachmentTemplate;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}

