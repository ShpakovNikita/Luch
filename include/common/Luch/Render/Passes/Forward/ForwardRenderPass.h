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
#include <Luch/Render/Passes/Forward/ForwardForwards.h>

namespace Luch::Render::Passes::Forward
{
    using namespace Graphics;
    using namespace Graph;

    class ForwardRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr Format LuminanceFormat = Format::RGBA16Sfloat;
    public:
        static const String RenderPassName;
        static const String RenderPassNameWithDepthOnly;

        static ResultValue<bool, UniquePtr<ForwardPersistentContext>> PrepareForwardPersistentContext(
            GraphicsDevice* device,
            CameraResources* cameraResources,
            MaterialResources* materialResources,
            IndirectLightingResources* indirectLightingResources);

        static ResultValue<bool, UniquePtr<ForwardTransientContext>> PrepareForwardTransientContext(
            ForwardPersistentContext* persistentContext,
            RefPtr<DescriptorPool> descriptorPool);

        ForwardRenderPass(
            ForwardPersistentContext* persistentContext,
            ForwardTransientContext* transientContext,
            RenderGraphBuilder* builder);

        ~ForwardRenderPass();

        void PrepareScene();
        void UpdateScene();

        inline RenderMutableResource GetLuminanceTextureHandle() { return luminanceTextureHandle; }

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
        void UpdateIndirectLightingDescriptorSet(RenderGraphResourceManager* manager, DescriptorSet* descriptorSet);

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawScene(SceneV1::Scene* scene, RenderGraphResourceManager* manager, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        static const String& GetRenderPassName(bool useDepthPrepass);

        static RefPtr<GraphicsPipelineState> CreatePipelineState(
            SceneV1::Primitive* primitive,
            bool useDepthPrepass,
            ForwardPersistentContext* context);

        ForwardPersistentContext* persistentContext = nullptr;
        ForwardTransientContext* transientContext = nullptr;

        RenderMutableResource luminanceTextureHandle;
        RenderMutableResource depthStencilTextureHandle;

        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;

        UnorderedMap<SceneV1::Mesh*, RefPtr<DescriptorSet>> meshDescriptorSets;
        UnorderedMap<SceneV1::Camera*, RefPtr<DescriptorSet>> cameraDescriptorSets;
    };
}
