#pragma once

#include <Luch/Graphics/Size2.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/RefPtr.h>

namespace Luch::Render::Passes
{
    using namespace Graph;
    using namespace Graphics;

    struct TonemapPersistentContext
    {
        GraphicsDevice* device = nullptr;

        RefPtr<GraphicsPipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;

        RefPtr<RenderPass> renderPass;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding colorTextureBinding;

        RefPtr<DescriptorSetLayout> textureDescriptorSetLayout;

        RefPtr<ShaderProgram> vertexShader;
        RefPtr<ShaderProgram> fragmentShader;
    };

    struct TonemapTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<DescriptorSet> textureDescriptorSet;
        RenderResource inputHandle;
        RenderMutableResource outputHandle;
        Size2i outputSize;
    };
}
