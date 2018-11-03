#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/ResultValue.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/Common.h>
#include <Husky/Render/RenderContext.h>

namespace Husky::Render
{
    using namespace Graphics;

    class Renderer
    {
    public:
        Renderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(SharedPtr<RenderContext> aContext) { context = aContext; }

        virtual bool Initialize() = 0;
        virtual bool Deinitialize() = 0;
        virtual const String& GetRendererName() = 0;

        virtual void PrepareScene(SceneV1::Scene* scene) = 0;
        virtual void UpdateScene(SceneV1::Scene* scene) = 0;
    protected:
        virtual void PrepareCameraNode(SceneV1::Node* node) = 0;
        virtual void PrepareMeshNode(SceneV1::Node* node) = 0;
        virtual void PrepareLightNode(SceneV1::Node* node) = 0;
        virtual void PrepareNode(SceneV1::Node* node);

        virtual void PrepareMesh(SceneV1::Mesh* mesh) = 0;
        virtual void PreparePrimitive(SceneV1::Primitive* primitive) = 0;
        virtual void PrepareMaterial(SceneV1::PbrMaterial* mesh) = 0;
        virtual void PrepareLights(SceneV1::Scene* scene) = 0;

        virtual void UpdateNode(SceneV1::Node* node, const Mat4x4& transform);
        virtual void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform) = 0;
        virtual void UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform) = 0;
        virtual void UpdateLight(SceneV1::Light* light, const Mat4x4& transform) = 0;
        virtual void UpdateMaterial(SceneV1::PbrMaterial* material) = 0;

        virtual void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        virtual void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        virtual void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList) = 0;
        virtual void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList) = 0;

        SharedPtr<RenderContext> context;
    };
}
