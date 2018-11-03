#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/ResultValue.h>
#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetBinding.h>
#include <Husky/Graphics/Attachment.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/Common.h>
#include <Husky/Render/RenderContext.h>
#include <Husky/Render/Deferred/DeferredForwards.h>

#include <Husky/Render/SharedBuffer.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    class GBufferRenderer
    {
    public:
        static constexpr int32 SharedUniformBufferSize = 16 * 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr int32 OffscreenImageCount = 3;
        static const String RendererName;

        GBufferRenderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return renderContext; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        const SharedPtr<DeferredRendererResources>& GetDeferredResources() { return resources; }
        void SetDeferredResources(const SharedPtr<DeferredRendererResources>& aResources) { resources = aResources; }

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        void DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera);
    private:
        void PrepareCameraNode(SceneV1::Node* node);
        void PrepareMeshNode(SceneV1::Node* node);
        void PrepareLightNode(SceneV1::Node* node);
        void PrepareNode(SceneV1::Node* node);
        void PrepareMesh(SceneV1::Mesh* mesh);
        void PreparePrimitive(SceneV1::Primitive* primitive);
        void PrepareMaterial(SceneV1::PbrMaterial* mesh);

        void UpdateNode(SceneV1::Node* node, const Mat4x4& parentTransform);
        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform);
        void UpdateMaterial(SceneV1::PbrMaterial* material);

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        RefPtr<PipelineState> CreateGBufferPipelineState(SceneV1::Primitive* primitive);

        ResultValue<bool, UniquePtr<DeferredRendererResources>> PrepareResources();
        ResultValue<bool, UniquePtr<GBufferPassResources>> PrepareGBufferPassResources();
        ResultValue<bool, OffscreenTextures> CreateOffscreenTextures();

        SharedPtr<RenderContext> context;

        SharedPtr<DeferredResources> resources;
        UniquePtr<GBufferPassResources> gbuffer;

        Format baseColorFormat = Format::R8G8B8A8Unorm;
        Format normalMapFormat = Format::R32G32B32A32Sfloat;
        Format depthStencilFormat = Format::D24UnormS8Uint;
        float32 minDepth = 0.0;
        float32 maxDepth = 1.0;
    };
}
