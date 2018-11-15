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
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>

#include <Luch/Render/SharedBuffer.h>

namespace Luch::Render::Deferred
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
        ~GBufferRenderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        const SharedPtr<DeferredResources>& GetDeferredResources() { return commonResources; }
        void SetDeferredResources(const SharedPtr<DeferredResources>& aResources) { commonResources = aResources; }

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        GBufferTextures* DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera);
    private:
        void PrepareMeshNode(SceneV1::Node* node);
        void PrepareNode(SceneV1::Node* node);
        void PrepareMesh(SceneV1::Mesh* mesh);
        void PreparePrimitive(SceneV1::Primitive* primitive);
        void PrepareMaterial(SceneV1::PbrMaterial* mesh);

        void UpdateNode(SceneV1::Node* node);
        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform);
        void UpdateMaterial(SceneV1::PbrMaterial* material);

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        RefPtr<PipelineState> CreateGBufferPipelineState(SceneV1::Primitive* primitive);

        ResultValue<bool, UniquePtr<GBufferPassResources>> PrepareGBufferPassResources();
        ResultValue<bool, UniquePtr<GBufferTextures>> CreateGBufferTextures();

        SharedPtr<RenderContext> context;

        SharedPtr<DeferredResources> commonResources;
        UniquePtr<GBufferPassResources> resources;
        UniquePtr<GBufferTextures> gbuffer;

        Format baseColorFormat = Format::R8G8B8A8Unorm;
        Format normalMapFormat = Format::R32G32B32A32Sfloat;
        Format depthStencilFormat = Format::D24UnormS8Uint;
        float32 minDepth = 0.0;
        float32 maxDepth = 1.0;
    };
}
