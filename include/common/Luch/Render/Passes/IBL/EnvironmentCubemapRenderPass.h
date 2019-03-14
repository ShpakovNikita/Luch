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
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;
    using namespace Graph;

    class EnvironmentCubemapRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
    public:
        static constexpr Format LuminanceFormat = Format::RGBA16Sfloat;
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<EnvironmentCubemapPersistentContext>> PrepareEnvironmentCubemapPersistentContext(
            GraphicsDevice* device,
            CameraResources* cameraResources,
            MaterialResources* materialResources);

        static ResultValue<bool, UniquePtr<EnvironmentCubemapTransientContext>> PrepareEnvironmentCubemapTransientContext(
            EnvironmentCubemapPersistentContext* persistentContext,
            float32 zNear,
            float32 zFar,
            RefPtr<DescriptorPool> descriptorPool);

        EnvironmentCubemapRenderPass(
            EnvironmentCubemapPersistentContext* persistentContext,
            EnvironmentCubemapTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~EnvironmentCubemapRenderPass();

        void PrepareScene();
        void UpdateScene();

        inline RenderMutableResource GetEnvironmentLuminanceCubemapHandle() { return luminanceCubemapHandle; }
        inline RenderMutableResource GetEnvironmentDepthCubemapHandle() { return luminanceDepthHandle; }

        void ExecuteGraphicsPass(
            RenderGraphResourceManager* manager,
            GraphicsCommandList* commandList) override;
    private:
        void PrepareNode(SceneV1::Node* node);
        void PrepareMeshNode(SceneV1::Node* node);
        void PrepareMesh(SceneV1::Mesh* mesh);
        void PreparePrimitive(SceneV1::Primitive* primitive);

        void UpdateNode(SceneV1::Node* node);
        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawScene(SceneV1::Scene* scene, int16 face, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        static RefPtr<GraphicsPipelineState> CreatePipelineState(
            SceneV1::Primitive* primitive,
            EnvironmentCubemapPersistentContext* context);

        EnvironmentCubemapPersistentContext* persistentContext = nullptr;
        EnvironmentCubemapTransientContext* transientContext = nullptr;

        RenderMutableResource luminanceCubemapHandle;
        RenderMutableResource luminanceDepthHandle;

        UnorderedMap<SceneV1::Mesh*, RefPtr<DescriptorSet>> meshDescriptorSets;
    };
}
