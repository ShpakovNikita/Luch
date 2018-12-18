#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetBinding.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/ResolvePassResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;
    using namespace Graph;

    class ResolveRenderPass : public RenderGraphPass
    {
    public:
        static constexpr int32 SharedUniformBufferSize = 16 * 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr int32 OffscreenImageCount = 3;
        static const String RenderPassName;

        ResolveRenderPass(RenderGraphBuilder* builder);
        ~ResolveRenderPass();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);

        void ExecuteRenderPass(
            RenderGraphResourceManager* manager,
            FrameBuffer* frameBuffer, 
            GraphicsCommandList* commandList) override;
    private:
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        RefPtr<PipelineState> CreateResolvePipelineState(ResolvePassResources* lighting);
        static ResultValue<bool, UniquePtr<ResolvePassResources>> PrepareResolvePassResources(RenderContext* context);

        SharedPtr<RenderContext> context;
        ResolvePassResources* resources;
    };
}
