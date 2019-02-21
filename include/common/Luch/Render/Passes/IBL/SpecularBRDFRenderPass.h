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

    class SpecularBRDFRenderPass : public RenderGraphPass
    {
        static constexpr int32 SharedUniformBufferSize = 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr Size3i ThreadsPerThreadgroup = { 16, 16, 1 };
        static constexpr Format BRDFFormat = Format::RGBA16Sfloat;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<SpecularBRDFPersistentContext>> PrepareSpecularBRDFPersistentContext(GraphicsDevice* device);

        static ResultValue<bool, UniquePtr<SpecularBRDFTransientContext>> PrepareSpecularBRDFTransientContext(
            SpecularBRDFPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        SpecularBRDFRenderPass(
            SpecularBRDFPersistentContext* persistentContext,
            SpecularBRDFTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~SpecularBRDFRenderPass();

        RenderMutableResource GetBRDFTextureHandle() { return brdfTextureHandle; }

        void ExecuteComputePass(
            RenderGraphResourceManager* manager,
            ComputeCommandList* commandList) override;
    private:
        void ComputeBRDF(
            RenderGraphResourceManager* manager,
            ComputeCommandList* commandList);

        static RefPtr<ComputePipelineState> CreateBRDFPipelineState(SpecularBRDFPersistentContext* context);

        SpecularBRDFPersistentContext* persistentContext = nullptr;
        SpecularBRDFTransientContext* transientContext = nullptr;

        RenderMutableResource brdfTextureHandle;
    };
}
