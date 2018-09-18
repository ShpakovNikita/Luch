#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Format.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>
#include <Husky/Vulkan/GlslShaderCompiler.h>
#include <Husky/Vulkan/DescriptorSetBinding.h>
#include <Husky/Render/Common.h>

namespace Husky::Render
{
    using namespace Husky::Vulkan;

    struct FrameResources
    {
        RefPtr<Framebuffer> framebuffer;
        RefPtr<DescriptorSet> cameraDescriptorSet;
        RefPtr<CommandPool> graphicsCommandPool;

        RefPtr<CommandBuffer> commandBuffer;
        RefPtr<Image> depthStencilBuffer;
        RefPtr<ImageView> depthStencilBufferView;

        RefPtr<DeviceBuffer> indexBuffer;
        RefPtr<DeviceBuffer> vertexBuffer;
        RefPtr<Fence> fence;
        RefPtr<Semaphore> semaphore;
        RefPtr<DeviceBuffer> cameraUniformBuffer;
    };

    struct ForwardRendererContext
    {
        RefPtr<PhysicalDevice> physicalDevice;
        RefPtr<Surface> surface;
        RefPtr<GraphicsDevice> device;
        RefPtr<Swapchain> swapchain;
        RefPtr<CommandPool> presentCommandPool;
        GLSLShaderCompiler shaderCompiler;
    };

    struct GlobalDrawContext
    {
        Mat4x4 model;
        Mat4x4 view;
        Mat4x4 projection;
        int32 frameIndex;
        FrameResources* frameResources;
        UnorderedMap<vk::DescriptorType, int32> descriptorCount;
    };

    struct PreparedScene
    {
        RefPtr<SceneV1::Scene> scene;
        RefPtr<SceneV1::Node> cameraNode;

        RefPtrVector<SceneV1::Light> lights;

        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<RenderPass> renderPass;
        RefPtr<PipelineLayout> pipelineLayout;

        DescriptorSetBinding cameraUniformBufferBinding;
        DescriptorSetBinding lightsStorageBufferBinding;
        DescriptorSetBinding meshUniformBufferBinding;
        DescriptorSetBinding baseColorTextureBinding;
        DescriptorSetBinding metallicRoughnessTextureBinding;
        DescriptorSetBinding normalTextureBinding;
        DescriptorSetBinding occlusionTextureBinding;
        DescriptorSetBinding emissiveTextureBinding;

        RefPtr<DeviceBuffer> lightsBuffer;
        RefPtr<DescriptorSet> lightsDescriptorSet;

        RefPtr<DescriptorSetLayout> cameraDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> lightsDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> meshDescriptorSetLayout;
        RefPtr<DescriptorSetLayout> materialDescriptorSetLayout;
        RefPtr<CommandPool> commandPool;
        GLSLShaderCompiler::Bytecode vertexShaderBytecode;
        GLSLShaderCompiler::Bytecode fragmentShaderBytecode;
        RefPtr<ShaderModule> vertexShaderModule;
        RefPtr<ShaderModule> fragmentShaderModule;
        Vector<FrameResources> frameResources;
    };

    class ForwardRenderer
    {
    public:
        ForwardRenderer(PhysicalDevice* physicalDevice, Surface* surface, int32 width, int32 height);

        bool Initialize();
        bool Deinitialize();

        ResultValue<bool, PreparedScene> PrepareScene(const RefPtr<SceneV1::Scene>& scene);
        void UpdateScene(PreparedScene& scene);
        void DrawScene(const PreparedScene& scene);

    private:
        void PrepareCameraNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene);
        void PrepareMeshNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene);
        void PrepareLightNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene);
        void PrepareNode(const RefPtr<SceneV1::Node>& node, PreparedScene& scene);
        void PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh, PreparedScene& scene);
        void PrepareLight(const RefPtr<SceneV1::Light>& light, PreparedScene& scene);
        void PrepareMaterial(const RefPtr<SceneV1::PbrMaterial>& mesh, PreparedScene& scene);

        void PrepareLights(PreparedScene& scene);

        void UpdateNode(const RefPtr<SceneV1::Node>& node, const Mat4x4& parentTransform, PreparedScene& scene);
        void UpdateMesh(const RefPtr<SceneV1::Mesh>& mesh, const Mat4x4& transform, PreparedScene& scene);
        void UpdateCamera(const RefPtr<SceneV1::Camera>& camera, const Mat4x4& transform, PreparedScene& scene);
        void UpdateLight(const RefPtr<SceneV1::Light>& light, const Mat4x4& transform, PreparedScene& scene);
        //void UpdateMaterial(const RefPtr<SceneV1::PbrMaterial>& material, PreparedScene& scene);

        void DrawNode(const RefPtr<SceneV1::Node>& node, const PreparedScene& scene, CommandBuffer* cmdBuffer);
        void DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, const PreparedScene& scene, CommandBuffer* cmdBuffer);
        void DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, const PreparedScene& scene, CommandBuffer* cmdBuffer);

        ImageDescriptorInfo ToVulkanImageDescriptorInfo(const SceneV1::TextureInfo& textureInfo);

        MaterialPushConstants GetMaterialPushConstants(SceneV1::PbrMaterial *material);

        Vector<Byte> LoadShaderSource(const FilePath& path);

        RefPtr<Pipeline> CreatePipeline(const RefPtr<SceneV1::Primitive>& primitive, PreparedScene& scene);

        Vector<const char8*> GetRequiredDeviceExtensionNames() const;

        UniquePtr<ForwardRendererContext> context;

        vk::AllocationCallbacks allocationCallbacks;

        Husky::int32 width;
        Husky::int32 height;
        Format depthStencilFormat = Format::D24UnormS8Uint;
        Husky::float32 minDepth = 0.0;
        Husky::float32 maxDepth = 1.0;
        Husky::int32 frameIndex = 0;
    };
}
