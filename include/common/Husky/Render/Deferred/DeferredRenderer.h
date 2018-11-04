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
#include <Husky/Render/ShaderDefines.h>
#include <Husky/Render/RenderContext.h>
#include <Husky/Render/Deferred/DeferredResources.h>
#include <Husky/Render/Deferred/GBufferPassResources.h>
#include <Husky/Render/Deferred/LightingPassResources.h>
#include <Husky/Render/Deferred/ResolvePassResources.h>
#include <Husky/Render/Deferred/DeferredOptions.h>
#include <Husky/Render/Deferred/DeferredShaderDefines.h>

#include <Husky/Render/SharedBuffer.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowRenderer.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingPassResources.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;
    using namespace ShadowMapping;

    class DeferredRenderer
    {
    public:
        static constexpr int32 SharedBufferSize = 1024 * 1024;
        static const String RendererName;

        DeferredRenderer(
            const RefPtr<PhysicalDevice>& physicalDevice,
            const RefPtr<Surface>& surface,
            int32 width,
            int32 height);

        ~DeferredRenderer();

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        void DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera);
    private:
        void PrepareCameraNode(SceneV1::Node* node);
        void PrepareNode(SceneV1::Node* node);

        void UpdateNodeRecursive(SceneV1::Node* node, const Mat4x4& transform);
        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform);

        ResultValue<bool, UniquePtr<DeferredResources>> PrepareResources();

        SharedPtr<RenderContext> context;

        UniquePtr<GBufferRenderer> gbufferRenderer;
        UniquePtr<LightingRenderer> lightingRenderer;
        UniquePtr<ResolveRenderer> resolveRenderer;
        UniquePtr<ShadowRenderer> shadowRenderer;

        SharedPtr<DeferredResources> resources;

        DeferredOptions options;

        int32 width = 0;
        int32 height = 0;
        Format swapchainFormat = Format::B8G8R8A8Unorm;
        float32 minDepth = 0.0;
        float32 maxDepth = 1.0;
    };
}
