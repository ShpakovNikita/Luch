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

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding baseColorSamplerBinding;
        DescriptorSetBinding lightingMapBinding;
        DescriptorSetBinding lightingSamplerBinding;

        RefPtr<Sampler> baseColorSampler;
        RefPtr<Sampler> lightingMapSampler;

        RefPtr<DescriptorSet> resolveTextureDescriptorSet;
        RefPtr<DescriptorSet> resolveSamplerDescriptorSet;

        RefPtr<DescriptorSetLayout> resovleTextureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> resolveSamplerDescriptorSetLayout;

        ColorAttachment colorAttachmentTemplate;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;
    };
}

