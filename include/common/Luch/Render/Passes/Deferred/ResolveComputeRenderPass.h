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
#include <Luch/Render/Passes/Deferred/DeferredForwards.h>
#include <Luch/Render/Passes/Deferred/GBuffer.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>

namespace Luch::Render::Passes::Deferred
{
    using namespace Graphics;
    using namespace Graph;

    class ResolveComputeRenderPass : public RenderGraphPass
    {
        static constexpr int32 SharedUniformBufferSize = 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr Size3i ThreadsPerThreadgroup = { 16, 16, 1 };
        static constexpr Format ColorFormat = Format::RGBA16Sfloat;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<ResolveComputePersistentContext>> PrepareResolvePersistentContext(
            GraphicsDevice* device,
            CameraPersistentResources* cameraResources,
            IndirectLightingPersistentResources* indirectLightingResources,
            LightPersistentResources* lightResources);

        static ResultValue<bool, UniquePtr<ResolveComputeTransientContext>> PrepareResolveTransientContext(
            ResolveComputePersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        ResolveComputeRenderPass(
            ResolveComputePersistentContext* persistentContext,
            ResolveComputeTransientContext* transientContext);

        ~ResolveComputeRenderPass();

        void PrepareScene();
        void UpdateScene();

        RenderMutableResource GetLuminanceTextureHandle() { return luminanceTextureHandle; }

        void Initialize(RenderGraphBuilder* builder) override;

        void ExecuteComputePass(
            RenderGraphResourceManager* manager,
            ComputeCommandList* commandList) override;
    private:
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        void UpdateGBufferDescriptorSet(
            RenderGraphResourceManager* manager,
            DescriptorSet* descriptorSet);

        void UpdateIndirectLightingDescriptorSet(
            RenderGraphResourceManager* manager,
            DescriptorSet* descriptorSet);

        static RefPtr<ComputePipelineState> CreateResolvePipelineState(ResolveComputePersistentContext* context);

        ResolveComputePersistentContext* persistentContext = nullptr;
        ResolveComputeTransientContext* transientContext = nullptr;

        GBufferReadOnly gbuffer;
        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;

        RenderMutableResource luminanceTextureHandle;
    };
}
