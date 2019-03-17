#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Size3.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;
    using namespace Graph;

    class SpecularReflectionRenderPass : public RenderGraphPass
    {
        static constexpr int32 SharedUniformBufferSize = 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr Size3i ThreadsPerThreadgroup = { 16, 16, 1 };
        static constexpr Format SpecularReflectionFormat = Format::RGBA16Sfloat;
        static constexpr Format BRDFFormat = Format::RGBA16Sfloat;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<SpecularReflectionPersistentContext>> PrepareSpecularReflectionPersistentContext(GraphicsDevice* device);

        static ResultValue<bool, UniquePtr<SpecularReflectionTransientContext>> PrepareSpecularReflectionTransientContext(
            SpecularReflectionPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        SpecularReflectionRenderPass(
            SpecularReflectionPersistentContext* persistentContext,
            SpecularReflectionTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~SpecularReflectionRenderPass();

        RenderMutableResource GetSpecularReflectionCubemapHandle() { return finalSpecularReflectionCubemapHandle; }

        void ExecuteComputePass(
            RenderGraphResourceManager* manager,
            ComputeCommandList* commandList) override;

        void ExecuteCopyPass(
            RenderGraphResourceManager* manager,
            CopyCommandList* commandList) override;
    private:
        void ComputeSpecularReflection(
            int32 mipLevel,
            RenderGraphResourceManager* manager,
            ComputeCommandList* commandList);

        static RefPtr<ComputePipelineState> CreateSpecularReflectionPipelineState(SpecularReflectionPersistentContext* context);

        SpecularReflectionPersistentContext* persistentContext = nullptr;
        SpecularReflectionTransientContext* transientContext = nullptr;

        int32 mipmapLevelCount = 0;
        RenderResource luminanceCubemapHandle;
        Vector<RenderMutableResource> specularReflectionCubemapHandles;
        RenderMutableResource finalSpecularReflectionCubemapHandle;
    };
}
