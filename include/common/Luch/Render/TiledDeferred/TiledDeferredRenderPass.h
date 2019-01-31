#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/TiledDeferred/TiledDeferredForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>

namespace Luch::Render::TiledDeferred
{
    using namespace Graphics;
    using namespace Graph;

    class TiledDeferredRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<TiledDeferredPersistentContext>> PrepareTiledDeferredPersistentContext(
            GraphicsDevice* device,
            CameraResources* cameraResources,
            MaterialResources* materialResources);

        static ResultValue<bool, UniquePtr<TiledDeferredTransientContext>> PrepareTiledDeferredTransientContext(
            TiledDeferredPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        TiledDeferredRenderPass(
            TiledDeferredPersistentContext* persistentContext,
            TiledDeferredTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~TiledDeferredRenderPass();

        void PrepareScene();
        void UpdateScene();

        SceneV1::Node* GetCameraNode() { return cameraNode; }
        void SetCameraNode(SceneV1::Node* node){ cameraNode = node; }

        RenderMutableResource GetResolveTextureHandle() { return resolveTextureHandle; }

        void ExecuteGraphicsRenderPass(
            RenderGraphResourceManager* manager,
            GraphicsCommandList* commandList) override;
    private:
        void PrepareNode(SceneV1::Node* node);
        void PrepareMeshNode(SceneV1::Node* node);
        void PrepareCameraNode(SceneV1::Node* node);
        void PrepareMesh(SceneV1::Mesh* mesh);
        void PreparePrimitive(SceneV1::Primitive* primitive);

        void UpdateNode(SceneV1::Node* node);
        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        void DrawGBuffer(GraphicsCommandList* commandList);
        void Resolve(GraphicsCommandList* commandList);

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        static const String& GetRenderPassName(bool useDepthPrepass);

        static RefPtr<GraphicsPipelineState> CreateGBufferPipelineState(SceneV1::Primitive* primitive, TiledDeferredPersistentContext* context);
        static RefPtr<TiledPipelineState> CreateResolvePipelineState(TiledDeferredPersistentContext* context);

        TiledDeferredPersistentContext* persistentContext = nullptr;
        TiledDeferredTransientContext* transientContext = nullptr;

        RenderMutableResource resolveTextureHandle;

        UnorderedMap<SceneV1::Mesh*, RefPtr<DescriptorSet>> meshDescriptorSets;
        UnorderedMap<SceneV1::Camera*, RefPtr<DescriptorSet>> cameraDescriptorSets;
        SceneV1::Node* cameraNode = nullptr;
    };
}
