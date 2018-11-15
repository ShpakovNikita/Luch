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
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/Deferred/DeferredResources.h>
#include <Luch/Render/Deferred/DeferredOptions.h>
#include <Luch/Render/Deferred/DeferredShaderDefines.h>

#include <Luch/Render/SharedBuffer.h>

namespace Luch::Render::Deferred
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

        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateCamera(SceneV1::Camera* camera, const Mat4x4& transform);

        ResultValue<bool, UniquePtr<DeferredResources>> PrepareResources();

        SharedPtr<RenderContext> context;

        UniquePtr<GBufferRenderer> gbufferRenderer;
        UniquePtr<ResolveRenderer> resolveRenderer;
        UniquePtr<TonemapRenderer> tonemapRenderer;
        UniquePtr<ShadowRenderer> shadowRenderer;

        SharedPtr<DeferredResources> resources;

        DeferredOptions options;

        int32 width = 0;
        int32 height = 0;
        Format swapchainFormat = Format::B8G8R8A8Unorm_sRGB;
        float32 minDepth = 0.0;
        float32 maxDepth = 1.0;
    };
}
