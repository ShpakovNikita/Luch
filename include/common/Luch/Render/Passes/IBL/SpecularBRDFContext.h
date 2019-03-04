#pragma once

#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/Size2.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;

    struct SpecularBRDFPersistentContext
    {
        GraphicsDevice* device = nullptr;

        RefPtr<ComputePipelineState> brdfPipelineState;
        RefPtr<PipelineLayout> brdfPipelineLayout;
        DescriptorSetBinding brdfTextureBinding;
        RefPtr<DescriptorSetLayout> brdfDescriptorSetLayout;
        RefPtr<ShaderProgram> brdfKernelShader;
    };

    struct SpecularBRDFTransientContext
    {
        RefPtr<DescriptorPool> descriptorPool;
        Size2i outputSize;
        SharedPtr<SharedBuffer> sharedBuffer;
        RefPtr<DescriptorSet> brdfDescriptorSet;
    };
}
