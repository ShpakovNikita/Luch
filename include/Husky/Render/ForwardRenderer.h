#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/UniquePtr.h>
#include <Husky/Format.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan/GlslShaderCompiler.h>

namespace Husky::Render
{
    using namespace Husky::Vulkan;

#pragma pack(push)
#pragma pack(1)
    struct Uniform
    {
        Mat4x4 model;
        Mat4x4 view;
        Mat4x4 projection;
        Vec3 cameraPosition;
        float32 _padding0;
    };
    
    struct MaterialPushConstants
    {
        int32 hasBaseColorTexture;
        int32 hasMetallicRoughnessTexture;
        int32 hasNormalTexture;
        int32 hasOcclusionTexture;
        int32 hasEmissiveTexture;
        int32 isAlphaMask;
        float32 alphaCutoff;
        float32 metallicFactor;
        float32 roughnessFactor;
        float32 emissiveFactor;
        float32 normalScale;
        float32 occlusionStrength;
        Vec4 baseColorFactor;
    };
#pragma pack(pop)

    struct CommandPoolCreateResult
    {
        vk::ResultValue<vk::CommandPool> graphicsCommandPool;
        vk::ResultValue<vk::CommandPool> presentCommandPool;
        vk::ResultValue<vk::CommandPool> computeCommandPool;

        Vector<vk::CommandPool> uniqueCommandPools;
    };

    struct FrameResources
    {
        RefPtr<Framebuffer> framebuffer;
        //DescriptorPool descriptorPool;
        RefPtr<CommandPool> graphicsCommandPool;

        RefPtr<CommandBuffer> commandBuffer;
        RefPtr<Image> depthStencilBuffer;
        RefPtr<ImageView> depthStencilBufferView;
        
        RefPtr<Buffer> indexBuffer;
        RefPtr<Buffer> vertexBuffer;
        RefPtr<Fence> fence;
        RefPtr<Semaphore> semaphore;
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
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<RenderPass> renderPass;
        RefPtr<PipelineLayout> pipelineLayout;
        RefPtr<DescriptorSetLayout> descriptorSetLayout;
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

        void Draw(const RefPtr<SceneV1::Node>& node);
        void DrawScene(const RefPtr<SceneV1::Scene>& scene);

        ResultValue<bool, PreparedScene> PrepareScene(const RefPtr<SceneV1::Scene>& scene);
        void DrawScene(const PreparedScene& scene);
    private:
        void PrepareNode(const RefPtr<SceneV1::Node>& node);
        void PrepareMesh(const RefPtr<SceneV1::Mesh>& mesh);

        void DrawMesh(const RefPtr<SceneV1::Mesh>& mesh, GlobalDrawContext* drawContext, CommandBuffer* cmdBuffer);
        void DrawPrimitive(const RefPtr<SceneV1::Primitive>& primitive, GlobalDrawContext* drawContext, CommandBuffer* cmdBuffer);

        RefPtr<Pipeline> CreatePipeline(const RefPtr<SceneV1::Primitive>& primitive);

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