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
#include <Luch/Render/Common.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;
    using namespace Graph;

    class TonemapRenderPass : public RenderGraphPass
    {
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<TonemapPersistentContext>> PrepareTonemapPersistentContext(
            GraphicsDevice* device,
            Format swapchainFormat);

        static ResultValue<bool, UniquePtr<TonemapTransientContext>> PrepareTonemapTransientContext(
            TonemapPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        TonemapRenderPass(
            TonemapPersistentContext* persistentContext,
            TonemapTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~TonemapRenderPass();

        void PrepareScene();
        void UpdateScene();

        void ExecuteGraphicsRenderPass(
            RenderGraphResourceManager* manager,
            FrameBuffer* frameBuffer,
            GraphicsCommandList* commandList) override;
    private:
        static RefPtr<GraphicsPipelineState> CreateTonemapPipelineState(TonemapPersistentContext* persistentContext);

        TonemapPersistentContext* persistentContext = nullptr;
        TonemapTransientContext* transientContext = nullptr;
        RenderResource input;
        RenderMutableResource output;
    };
}