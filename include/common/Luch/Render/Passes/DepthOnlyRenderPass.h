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
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphPass.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Graph;

    class DepthOnlyRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
    public:
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<DepthOnlyPersistentContext>> PrepareDepthOnlyPersistentContext(
            GraphicsDevice* device,
            CameraResources* cameraResources,
            MaterialResources* materialResources);

        static ResultValue<bool, UniquePtr<DepthOnlyTransientContext>> PrepareDepthOnlyTransientContext(
            DepthOnlyPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        DepthOnlyRenderPass(
            DepthOnlyPersistentContext* persistentContext,
            DepthOnlyTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~DepthOnlyRenderPass();

        void PrepareScene();
        void UpdateScene();

        SceneV1::Node* GetCameraNode() { return cameraNode; }
        void SetCameraNode(SceneV1::Node* node){ cameraNode = node; }

        RenderMutableResource GetDepthTextureHandle() { return depthTextureHandle; }

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

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        RefPtr<GraphicsPipelineState> CreateDepthOnlyPipelineState(SceneV1::Primitive* primitive);

        DepthOnlyPersistentContext* persistentContext = nullptr;
        DepthOnlyTransientContext* transientContext = nullptr;
        RenderMutableResource depthTextureHandle;

        UnorderedMap<SceneV1::Mesh*, RefPtr<DescriptorSet>> meshDescriptorSets;
        UnorderedMap<SceneV1::Camera*, RefPtr<DescriptorSet>> cameraDescriptorSets;
        SceneV1::Node* cameraNode = nullptr;
    };
}
