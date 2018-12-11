#pragma once

#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/RefPtr.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    struct TonemapPassResources
    {
        RefPtr<PipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<CommandPool> commandPool;
        RefPtr<RenderPass> renderPass;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding colorTextureBinding;
        DescriptorSetBinding colorSamplerBinding;

        RefPtr<Sampler> sampler;

        RefPtr<DescriptorSetLayout> textureDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> samplerDescriptorSetLayout;

        RefPtr<DescriptorSet> textureDescriptorSet;
        RefPtr<DescriptorSet> samplerDescriptorSet;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;

        UniquePtr<SharedBuffer> sharedBuffer;
    };
}

