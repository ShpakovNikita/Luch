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
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/Deferred/GBuffer.h>
#include <Luch/Render/Deferred/DeferredConstants.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;
    using namespace Graph;

    class GBufferRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
    public:
        static const String RenderPassName;
        static const String RenderPassWithDepthOnlyName;

        static ResultValue<bool, UniquePtr<GBufferPersistentContext>> PrepareGBufferPersistentContext(
            GraphicsDevice* device,
            CameraResources* cameraResources,
            MaterialResources* materialResources);

        static ResultValue<bool, UniquePtr<GBufferTransientContext>> PrepareGBufferTransientContext(
            GBufferPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        GBufferRenderPass(
            GBufferPersistentContext* persistentContext,
            GBufferTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~GBufferRenderPass();

        void PrepareScene();
        void UpdateScene();

        SceneV1::Node* GetCameraNode() { return cameraNode; }
        void SetCameraNode(SceneV1::Node* node){ cameraNode = node; }

        GBufferReadOnly GetGBuffer() { return gbuffer; }

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

        static const String& GetRenderPassName(bool useDepthPrepass);

        RefPtr<GraphicsPipelineState> CreateGBufferPipelineState(
            SceneV1::Primitive* primitive,
            bool useDepthPrepass);

        GBufferPersistentContext* persistentContext = nullptr;
        GBufferTransientContext* transientContext = nullptr;
        GBuffer gbuffer;

        UnorderedMap<SceneV1::Mesh*, RefPtr<DescriptorSet>> meshDescriptorSets;
        UnorderedMap<SceneV1::Camera*, RefPtr<DescriptorSet>> cameraDescriptorSets;
        SceneV1::Node* cameraNode = nullptr;
    };
}
