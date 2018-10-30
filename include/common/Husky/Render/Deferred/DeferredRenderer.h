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

#include <Husky/Render/SharedBuffer.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowRenderer.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingPassResources.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;
    using namespace ShadowMapping;

    struct DeferredRendererResources
    {
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<CommandPool> commandPool;

        RefPtr<DescriptorSetLayout> cameraBufferDescriptorSetLayout;
        RefPtr<DescriptorSet> cameraBufferDescriptorSet;
        DescriptorSetBinding cameraUniformBufferBinding;

        UniquePtr<SharedBuffer> sharedBuffer;
    };

    class DeferredRenderer
    {
    public:
        static constexpr int32 SharedUniformBufferSize = 16 * 1024 * 1024;
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr int32 OffscreenImageCount = 3;
        static const String RendererName;

        DeferredRenderer(
            const RefPtr<PhysicalDevice>& physicalDevice,
            const RefPtr<Surface>& surface,
            int32 width,
            int32 height);

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(const RefPtr<SceneV1::Scene>& scene);
        void UpdateScene(const RefPtr<SceneV1::Scene>& scene);
        void DrawScene(const RefPtr<SceneV1::Scene>& scene, const RefPtr<SceneV1::Camera>& camera);
    private:
        void PrepareCameraNode(const RefPtr<SceneV1::Node>& node);
        void PrepareMeshNode(const RefPtr<SceneV1::Node>& node);
        void PrepareLightNode(const RefPtr<SceneV1::Node>& node);
        void PrepareNode(const RefPtr<SceneV1::Node>& node);
        void PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh);
        void PreparePrimitive(const RefPtr<SceneV1::Primitive>& primitive);
        void PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& mesh);
        void PrepareLights(const RefPtr<SceneV1::Scene>& scene);

        void UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform);
        void UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform);
        void UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform);
        void UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform);
        void UpdateMaterial(const RefPtr<SceneV1::PbrMaterial>& material);

        void DrawNode(
            const RefPtr<SceneV1::Node>& node,
            GraphicsCommandList* commandList);

        void DrawMesh(
            const RefPtr<SceneV1::Mesh>& mesh,
            GraphicsCommandList* commandList);

        void BindMaterial(
            const RefPtr<SceneV1::PbrMaterial>& material,
            GraphicsCommandList* commandList);

        void DrawPrimitive(
            const RefPtr<SceneV1::Primitive>& primitive,
            GraphicsCommandList* commandList);

        RefPtr<PipelineState> CreateGBufferPipelineState(const RefPtr<SceneV1::Primitive>& primitive);
        RefPtr<PipelineState> CreateLightingPipelineState(LightingPassResources* lighting);

        Vector<const char8*> GetRequiredDeviceExtensionNames() const;

        ResultValue<bool, UniquePtr<DeferredRendererResources>> PrepareResources();

        ResultValue<bool, UniquePtr<GBufferPassResources>> PrepareGBufferPassResources();

        ResultValue<bool, UniquePtr<LightingPassResources>> PrepareLightingPassResources(
            GBufferPassResources* gbufferResources);

        ResultValue<bool, OffscreenTextures> CreateOffscreenTextures();

        SharedPtr<RenderContext> context;

        UniquePtr<ShadowRenderer> shadowRenderer;

        UniquePtr<DeferredRendererResources> resources;
        UniquePtr<GBufferPassResources> gbuffer;
        UniquePtr<LightingPassResources> lighting;

        DeferredOptions options;

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
