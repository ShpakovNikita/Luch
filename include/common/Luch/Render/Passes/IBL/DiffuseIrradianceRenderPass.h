#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/Size3.h>
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

    class DiffuseIrradianceRenderPass : public RenderGraphPass
    {
        static constexpr int32 SharedUniformBufferSize = 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr Size3i ThreadsPerThreadgroup = { 16, 16, 1 };
        static constexpr Format IrradianceFormat = Format::RGBA8Unorm;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<DiffuseIrradiancePersistentContext>> PrepareDiffuseIrradiancePersistentContext(GraphicsDevice* device);

        static ResultValue<bool, UniquePtr<DiffuseIrradianceTransientContext>> PrepareDiffuseIrradianceTransientContext(
            DiffuseIrradiancePersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        DiffuseIrradianceRenderPass(
            DiffuseIrradiancePersistentContext* persistentContext,
            DiffuseIrradianceTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~DiffuseIrradianceRenderPass();

        RenderMutableResource GetIrradianceCubemapHandle() { return irradianceCubemapHandle; }

        void ExecuteComputePass(
            RenderGraphResourceManager* manager,
            ComputeCommandList* commandList) override;
    private:
        static RefPtr<ComputePipelineState> CreateDiffuseIrradiancePipelineState(DiffuseIrradiancePersistentContext* context);

        DiffuseIrradiancePersistentContext* persistentContext = nullptr;
        DiffuseIrradianceTransientContext* transientContext = nullptr;

        RenderResource luminanceCubemapHandle;
        RenderMutableResource irradianceCubemapHandle;
    };
}
