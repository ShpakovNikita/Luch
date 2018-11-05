#pragma once

#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct TonemapPassResources
    {
        RefPtr<PipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<CommandPool> commandPool;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding colorTextureBinding;
        DescriptorSetBinding colorSamplerBinding;

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

