#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
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

    class ResolveRenderPass : public RenderGraphPass
    {
        static constexpr Format ColorFormat = Format::RGBA16Sfloat;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<ResolvePersistentContext>> PrepareResolvePersistentContext(
            GraphicsDevice* device,
            CameraResources* cameraResources);

        static ResultValue<bool, UniquePtr<ResolveTransientContext>> PrepareResolveTransientContext(
            ResolvePersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        ResolveRenderPass(
            ResolvePersistentContext* persistentContext,
            ResolveTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~ResolveRenderPass();

        void PrepareScene();
        void UpdateScene();

        RenderMutableResource GetResolveTextureHandle() { return resolveTextureHandle; }

        SceneV1::Camera* GetCamera() { return camera; }
        void SetCamera(SceneV1::Camera* aCamera) { camera = aCamera; }

        void ExecuteGraphicsRenderPass(
            RenderGraphResourceManager* manager,
            GraphicsCommandList* commandList) override;
    private:
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        static RefPtr<GraphicsPipelineState> CreateResolvePipelineState(ResolvePersistentContext* context);

        SceneV1::Camera* camera = nullptr;
        ResolvePersistentContext* persistentContext = nullptr;
        ResolveTransientContext* transientContext = nullptr;
        GBufferReadOnly gbuffer;

        RenderMutableResource resolveTextureHandle;
    };
}
