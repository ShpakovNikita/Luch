#pragma once

#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/RenderForwards.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingPassResources.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingOptions.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/ResultValue.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    class ShadowMappingOptions;

    class ShadowRenderer
    {
    public:
        static const String RendererName;
        static constexpr int32 SharedUniformBufferSize = 4 * 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 1024;
        static constexpr int32 MaxDescriptorCount = 1024;
        // unordered map stores vectors of depth textures for each light
        // vector stores either one (for directional and spot lights) or six (for point lights) depth textures
        using ShadowMaps = UnorderedMap<SceneV1::Light*, RefPtrVector<Texture>>;

        ShadowRenderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        bool Initialize();
        bool Deinitialize();

        const ShadowMappingOptions& GetOptions() const { return options; }
        void SetOptions(const ShadowMappingOptions& aOptions) { options = aOptions; }

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        const ShadowMaps& DrawShadows(SceneV1::Scene* scene, const RefPtrVector<SceneV1::Node>& lightNodes);
    private:
        RefPtrVector<Texture> DrawSceneForLight(
            SceneV1::Light* light,
            Mat4x4 transform,
            SceneV1::Scene* scene,
            GraphicsCommandList* commandList);

        RefPtr<Texture> DrawSceneForCamera(
            SceneV1::Camera* camera,
            SceneV1::Scene* scene,
            GraphicsCommandList* commandList);

        void DrawNode(
            SceneV1::Node* node,
            GraphicsCommandList* commandList);

        void DrawMesh(
            SceneV1::Mesh* mesh,
            GraphicsCommandList* commandList);

        void DrawPrimitive(
            SceneV1::Primitive* primitive,
            GraphicsCommandList* commandList);

        void PrepareCamera(SceneV1::Camera* camera);
        void PrepareMesh(SceneV1::Mesh* mesh);

        void PrepareMeshNode(SceneV1::Node* meshNode);

        void UpdateNode(SceneV1::Node* node);
        void UpdateMesh(SceneV1::Mesh* mesh, Mat4x4 transform);

        void UpdateSpotLightCamera(
            SceneV1::Light* light,
            Mat4x4 transform,
            SceneV1::PerspectiveCamera* camera);

        void UpdateDirectionalLightCamera(
            SceneV1::Light* light,
            Mat4x4 transform,
            SceneV1::OrthographicCamera* camera);

        void UpdatePointLightCamera(
            SceneV1::Light* light,
            Mat4x4 transform,
            SceneV1::PerspectiveCamera* camera,
            int32 index);

        void UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform);

        ResultValue<bool, UniquePtr<ShadowMappingPassResources>> PrepareShadowMappingPassResources();

        SharedPtr<RenderContext> context;
        UniquePtr<ShadowMappingPassResources> resources;
        ShadowMappingOptions options;

        ShadowMaps shadowMaps;
        UnorderedMap<SceneV1::Light*, RefPtr<SceneV1::PerspectiveCamera>> spotLightCameras;
        UnorderedMap<SceneV1::Light*, RefPtr<SceneV1::OrthographicCamera>> directionalLightCameras;
        UnorderedMap<SceneV1::Light*, RefPtrArray<SceneV1::PerspectiveCamera, 6>> pointLightCameras;
    };
}
