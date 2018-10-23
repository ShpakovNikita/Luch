#pragma once

#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/RenderContext.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingPassResources.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingOptions.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/ResultValue.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    using namespace Graphics;

    class ShadowMappingOptions;

    class ShadowRenderer
    {
    public:
        // unordered map stores vectors of depth textures for each light
        // vector stores either one (for directional and spot lights) or six (for point lights) depth textures
        using ShadowMaps = UnorderedMap<SceneV1::Light*, RefPtrVector<Texture>>;

        ShadowRenderer(SharedPtr<RenderContext> context);

        bool Prepare(SceneV1::Scene* scene);

        const ShadowMappingOptions& GetOptions() const { return options; }
        void SetOptions(const ShadowMappingOptions& aOptions) { options = aOptions; }

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

        void UpdateCameraBuffer(SceneV1::Camera* camera);

        ResultValue<bool, ShadowMappingPassResources> PrepareShadowMappingPassResources(SceneV1::Scene* scene);

        SharedPtr<RenderContext> context;
        ShadowMappingPassResources resources;
        ShadowMappingOptions options;

        ShadowMaps shadowMaps;
        UnorderedMap<SceneV1::Light*, RefPtr<SceneV1::PerspectiveCamera>> spotLightCameras;
        UnorderedMap<SceneV1::Light*, RefPtr<SceneV1::OrthographicCamera>> directionalLightCameras;
        UnorderedMap<SceneV1::Light*, RefPtrArray<SceneV1::PerspectiveCamera, 6>> pointLightCameras;
    };
}
