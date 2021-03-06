#pragma once

#include <Luch/Size2.h>
#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;

    struct DiffuseIlluminancePersistentContext
    {
        GraphicsDevice* device = nullptr;

        RefPtr<ComputePipelineState> pipelineState;
        RefPtr<PipelineLayout> pipelineLayout;

        DescriptorSetBinding luminanceCubemapBinding;
        DescriptorSetBinding illuminanceCubemapBinding;

        RefPtr<DescriptorSetLayout> cubemapDescriptorSetLayout;

        RefPtr<ShaderProgram> kernelShader;
    };

    struct DiffuseIlluminanceTransientContext
    {
        SceneV1::Scene* scene = nullptr;
        RefPtr<DescriptorPool> descriptorPool;
        Size2i outputSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        RenderResource luminanceCubemapHandle;
        RefPtr<DescriptorSet> cubemapDescriptorSet;
    };
}
