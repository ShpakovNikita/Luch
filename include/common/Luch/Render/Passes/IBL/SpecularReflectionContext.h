#pragma once

#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/RefPtr.h>
#include <Luch/SharedPtr.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;

    struct SpecularReflectionPersistentContext
    {
        GraphicsDevice* device = nullptr;

        RefPtr<ComputePipelineState> specularReflectionPipelineState;
        RefPtr<PipelineLayout> specularReflectionPipelineLayout;
        DescriptorSetBinding luminanceCubemapBinding;
        DescriptorSetBinding specularReflectionCubemapBinding;
        DescriptorSetBinding specularReflectionParamsBufferBinding;
        RefPtr<DescriptorSetLayout> cubemapDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> paramsBufferDescriptorSetLayout;
        RefPtr<ShaderProgram> specularReflectionKernelShader;
    };

    struct SpecularReflectionTransientContext
    {
        RefPtr<DescriptorPool> descriptorPool;
        Size2i outputSize;
        int32 mipLevel = 0;
        SharedPtr<SharedBuffer> sharedBuffer;
        RenderResource luminanceCubemapHandle;
        RefPtr<DescriptorSet> cubemapDescriptorSet;
        RefPtr<DescriptorSet> paramsBufferDescriptorSet;
    };
}
