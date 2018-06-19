#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Format.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan/Attachment.h>
#include <Husky/Vulkan/GlslShaderCompiler.h>
#include <Husky/Vulkan/DescriptorSetBinding.h>
#include <Husky/Render/Common.h>

namespace Husky::Render
{
    using namespace Husky::Vulkan;

    struct QuadVertex
    {
        Vec3 position;
        Vec2 texCoord;
    };

    struct OffscreenImages
    {
        RefPtr<Image> depthStencilBuffer;
        RefPtr<ImageView> depthStencilBufferView;

        RefPtr<Image> baseColorImage;
        RefPtr<ImageView> baseColorImageView;

        RefPtr<Image> normalMapImage;
        RefPtr<ImageView> normalMapImageView;
    };

    struct DeferredFrameResources
    {
        RefPtr<Framebuffer> offscreenFramebuffer;
        RefPtr<Framebuffer> framebuffer;

        RefPtr<DescriptorSet> cameraDescriptorSet;
        RefPtr<DescriptorSet> gbufferDescriptorSet;
        RefPtr<Buffer> cameraUniformBuffer;

        RefPtr<CommandPool> graphicsCommandPool;

        OffscreenImages offscreen;

        RefPtr<Buffer> indexBuffer;
        RefPtr<Buffer> vertexBuffer;

        RefPtr<Fence> fence;
        RefPtr<Semaphore> drawSemaphore;
        RefPtr<Semaphore> offscreenSemaphore;

        RefPtr<CommandBuffer> gBufferCommandBuffer;
        RefPtr<CommandBuffer> lightingCommandBuffer;
    };

    struct DeferredRendererContext
    {
        RefPtr<PhysicalDevice> physicalDevice;
        RefPtr<Surface> surface;
        RefPtr<GraphicsDevice> device;
        RefPtr<Swapchain> swapchain;
        RefPtr<CommandPool> presentCommandPool;
        GLSLShaderCompiler shaderCompiler;
    };

    struct DeferredDrawContext
    {
        Mat4x4 model;
        Mat4x4 view;
        Mat4x4 projection;
        int32 frameIndex;
        DeferredFrameResources* frameResources;
        UnorderedMap<vk::DescriptorType, int32> descriptorCount;
    };

    struct GBufferPassResources
    {
        RefPtr<RenderPass> renderPass;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        DescriptorSetBinding meshUniformBufferBinding;
        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding metallicRoughnessTextureBinding;
        DescriptorSetBinding normalTextureBinding;
        DescriptorSetBinding occlusionTextureBinding;
        DescriptorSetBinding emissiveTextureBinding;

        Attachment baseColorAttachment;
        Attachment normalMapAttachment;
        Attachment depthStencilAttachment;

        RefPtr<DescriptorSetLayout> meshDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialDescriptorSetLayout;

        Shader vertexShader;
        Shader fragmentShader;
    };

    struct LightingPassResources
    {
        RefPtr<RenderPass> renderPass;
        RefPtr<Pipeline> pipeline;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        RefPtr<Buffer> fullscreenQuadBuffer;

        DescriptorSetBinding lightsUniformBufferBinding;
        RefPtr<Buffer> lightsBuffer;

        DescriptorSetBinding baseColorImageBinding;
        DescriptorSetBinding normalMapImageBinding;
        //DescriptorSetBinding depthStencilBufferBinding;

        RefPtr<Vulkan::Sampler> baseColorSampler;
        RefPtr<Vulkan::Sampler> normalMapSampler;

        RefPtr<DescriptorSetLayout> gbufferDescriptorSetLayout;

        RefPtr<DescriptorSet> lightsDescriptorSet;
        RefPtr<DescriptorSetLayout> lightsDescriptorSetLayout;

        Attachment colorAttachment;

        Shader vertexShader;
        Shader fragmentShader;
    };

    struct DeferredPreparedScene
    {
        RefPtr<SceneV1::Scene> scene;
        RefPtr<SceneV1::Node> cameraNode;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorPool> descriptorPool;

        DescriptorSetBinding cameraUniformBufferBinding;
        RefPtr<DescriptorSetLayout> cameraDescriptorSetLayout;

        RefPtrVector<SceneV1::Light> lights;

        RefPtr<CommandPool> commandPool;

        GBufferPassResources gBuffer;
        LightingPassResources lighting;

        Vector<DeferredFrameResources> frameResources;
    };

    class DeferredRenderer
    {
    public:
        static constexpr int32 OffscreenImagesCount = 3;

        DeferredRenderer(PhysicalDevice* physicalDevice, Surface* surface, int32 width, int32 height);

        bool Initialize();
        bool Deinitialize();

        ResultValue<bool, DeferredPreparedScene> PrepareScene(const RefPtr<SceneV1::Scene>& scene);
        void UpdateScene(DeferredPreparedScene& scene);
        void DrawScene(const DeferredPreparedScene& scene);

    private:
        void PrepareCameraNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareMeshNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareLightNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareNode(const RefPtr<SceneV1::Node>& node, DeferredPreparedScene& scene);
        void PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, DeferredPreparedScene& scene);
        void PrepareLight(const RefPtr<SceneV1::Light>& light, DeferredPreparedScene& scene);
        void PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& mesh, DeferredPreparedScene& scene);

        void PrepareLights(DeferredPreparedScene& scene);

        void UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform, DeferredPreparedScene& scene);
        void UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform, DeferredPreparedScene& scene);
        void UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform, DeferredPreparedScene& scene);
        void UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform, DeferredPreparedScene& scene);
        //void UpdateMaterial(const RefPtr<SceneV1::PbrMaterial>& material, PreparedScene& scene);

        void DrawNode(const RefPtr<SceneV1::Node>& node, const DeferredPreparedScene& scene, CommandBuffer* cmdBuffer);
        void DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, const DeferredPreparedScene& scene, CommandBuffer* cmdBuffer);
        void DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, const DeferredPreparedScene& scene, CommandBuffer* cmdBuffer);

        ImageDescriptorInfo ToVulkanImageDescriptorInfo(const SceneV1::TextureInfo& textureInfo);

        MaterialPushConstants GetMaterialPushConstants(SceneV1::PbrMaterial *material);

        Vector<Byte> LoadShaderSource(const FilePath& path);

        RefPtr<Pipeline> CreateGBufferPipeline(const RefPtr<SceneV1::Primitive>& primitive, DeferredPreparedScene& scene);
        RefPtr<Pipeline> CreateLightingPipeline(const LightingPassResources& lighting);

        Vector<const char8*> GetRequiredDeviceExtensionNames() const;

        ResultValue<bool, GBufferPassResources> PrepareGBufferPassResources(DeferredPreparedScene& scene);
        ResultValue<bool, LightingPassResources> PrepareLightingPassResources(DeferredPreparedScene& scene);
        ResultValue<bool, OffscreenImages> CreateOffscreenImages();
        ResultValue<bool, Shader> CreateShader(ShaderStage stage, const String& path);

        UniquePtr<DeferredRendererContext> context;

        vk::AllocationCallbacks allocationCallbacks;

        Husky::int32 width;
        Husky::int32 height;
        Format depthStencilFormat = Format::D24UnormS8Uint;
        Husky::float32 minDepth = 0.0;
        Husky::float32 maxDepth = 1.0;
        Husky::int32 frameIndex = 0;
    };
}