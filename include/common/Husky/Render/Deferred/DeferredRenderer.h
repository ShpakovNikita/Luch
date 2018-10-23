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
#include <Husky/Render/Deferred/GBufferPassResources.h>
#include <Husky/Render/Deferred/LightingPassResources.h>
#include <Husky/Render/Deferred/DeferredOptions.h>
#include <Husky/Render/Deferred/DeferredShaderDefines.h>

#include <Husky/Render/Deferred/ShadowMapping/ShadowRenderer.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingPassResources.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;
    using namespace ShadowMapping;

    struct QuadVertex
    {
        Vec3 position;
        Vec2 texCoord;
    };

    struct DeferredFrameResources
    {
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        RefPtr<DescriptorSet> gbufferTextureDescriptorSet;
        RefPtr<DescriptorSet> gbufferSamplerDescriptorSet;
        RefPtr<Buffer> cameraUniformBuffer;

        RefPtr<CommandPool> commandPool;
    };

    struct DeferredPreparedScene
    {
        RefPtr<SceneV1::Scene> scene;
        RefPtr<SceneV1::Node> cameraNode;
        RefPtrVector<SceneV1::Node> lightNodes;

        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<CommandPool> commandPool;

        GBufferPassResources gbuffer;
        LightingPassResources lighting;

        Vector<DeferredFrameResources> frameResources;
        DeferredOptions options;
    };

    class DeferredRenderer
    {
    public:
        static constexpr int32 OffscreenImagesCount = 3;

        DeferredRenderer(
            const RefPtr<PhysicalDevice>& physicalDevice,
            const RefPtr<Surface>& surface,
            int32 width,
            int32 height);

        bool Initialize();
        bool Deinitialize();

        ResultValue<bool, DeferredPreparedScene> PrepareScene(const RefPtr<SceneV1::Scene>& scene);
        void UpdateScene(DeferredPreparedScene& scene);
        void DrawScene(DeferredPreparedScene& scene);

    private:
        void PrepareCameraNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareMeshNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareLightNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, DeferredPreparedScene& scene);
        void PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& mesh, DeferredPreparedScene& scene);

        void PrepareLights(DeferredPreparedScene& scene);

        void UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform, DeferredPreparedScene& scene);
        void UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform, DeferredPreparedScene& scene);
        void UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform, DeferredPreparedScene& scene);
        void UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform, DeferredPreparedScene& scene);
        //void UpdateMaterial(const RefPtr<SceneV1::PbrMaterial>& material, PreparedScene& scene);

        void DrawNode(
            const RefPtr<SceneV1::Node>& node,
            DeferredPreparedScene& scene,
            GraphicsCommandList* commandList);

        void DrawMesh(
            const RefPtr<SceneV1::Mesh>& mesh,
            DeferredPreparedScene& scene,
            GraphicsCommandList* commandList);

        void BindMaterial(
            const RefPtr<SceneV1::PbrMaterial>& material,
            DeferredPreparedScene& scene,
            GraphicsCommandList* commandList);

        void DrawPrimitive(
            const RefPtr<SceneV1::Primitive>& primitive,
            DeferredPreparedScene& scene,
            GraphicsCommandList* commandList);

        RefPtr<PipelineState> CreateGBufferPipelineState(
            const RefPtr<SceneV1::Primitive>& primitive,
            DeferredPreparedScene& scene);

        RefPtr<PipelineState> CreateLightingPipelineState(const LightingPassResources& lighting);

        Vector<const char8*> GetRequiredDeviceExtensionNames() const;

        ResultValue<bool, GBufferPassResources> PrepareGBufferPassResources(DeferredPreparedScene& scene);
        ResultValue<bool, LightingPassResources> PrepareLightingPassResources(DeferredPreparedScene& scene);

        ResultValue<bool, OffscreenTextures> CreateOffscreenTextures();

        SharedPtr<RenderContext> context;
        UniquePtr<ShadowRenderer> shadowRenderer;

        int32 width = 0;
        int32 height = 0;
        Format swapchainFormat = Format::B8G8R8A8Unorm;
        Format baseColorFormat = Format::R8G8B8A8Unorm;
        Format normalMapFormat = Format::R32G32B32A32Sfloat;
        Format depthStencilFormat = Format::D24UnormS8Uint;
        float32 minDepth = 0.0;
        float32 maxDepth = 1.0;
        int32 frameIndex = 0;
    };
}
