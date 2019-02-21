#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/Passes/PassesForwards.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/CameraResources.h>
#include <Luch/Render/MaterialResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render
{
    using namespace Graphics;
    using namespace Passes;

    struct IBLResult
    {
        RefPtr<Texture> environmentCubemap;
        RefPtr<Texture> diffuseIrradianceCubemap;
        RefPtr<Texture> specularReflectionCubemap;
        RefPtr<Texture> specularBRDFTexture;
    };

    class IBLRenderer
    {
        static constexpr int32 DescriptorSetCount = 2048;
        static constexpr int32 DescriptorCount = 8192;
        static constexpr int32 SharedBufferSize = 1024 * 1024;
    public:
        IBLRenderer(RefPtr<SceneV1::Scene> scene);
        ~IBLRenderer();

        bool Initialize(
            SharedPtr<RenderContext> context,
            SharedPtr<MaterialManager> materialManager,
            SharedPtr<CameraResources> cameraResources);

        bool Deinitialize();

        bool BeginRender();
        void UpdateScene();
        bool PrepareScene();
        void ProbeIndirectLighting(Vec3 position);
        ResultValue<bool, IBLResult> EndRender();
    private:
        bool PrepareEnvironmentMapping();
        bool PrepareDiffuseIrradiance();
        bool PrepareSpecularReflection();
        bool PrepareSpecularBRDF();

        SharedPtr<SharedBuffer> sharedBuffer;

        UniquePtr<Graph::RenderGraphResourcePool> resourcePool;

        UniquePtr<IBL::EnvironmentCubemapPersistentContext> environmentCubemapPersistentContext;
        UniquePtr<IBL::DiffuseIrradiancePersistentContext> diffuseIrradiancePersistentContext;
        UniquePtr<IBL::SpecularReflectionPersistentContext> specularReflectionPersistentContext;
        UniquePtr<IBL::SpecularBRDFPersistentContext> specularBRDFPersistentContext;

        UniquePtr<IBL::EnvironmentCubemapTransientContext> environmentCubemapTransientContext;
        UniquePtr<IBL::DiffuseIrradianceTransientContext> diffuseIrradianceTransientContext;
        UniquePtr<IBL::SpecularReflectionTransientContext> specularReflectionTransientContext;
        UniquePtr<IBL::SpecularBRDFTransientContext> specularBRDFTransientContext;

        UniquePtr<IBL::EnvironmentCubemapRenderPass> environmentCubemapPass;
        UniquePtr<IBL::DiffuseIrradianceRenderPass> diffuseIrradiancePass;
        UniquePtr<IBL::SpecularReflectionRenderPass> specularReflectionPass;
        UniquePtr<IBL::SpecularBRDFRenderPass> specularBRDFPass;

        UniquePtr<Graph::RenderGraphBuilder> builder;
        UniquePtr<Graph::RenderGraph> renderGraph;

        SharedPtr<RenderContext> context;
        SharedPtr<MaterialManager> materialManager;
        SharedPtr<CameraResources> cameraResources;

        RefPtr<Semaphore> renderSemaphore;
        RefPtr<Semaphore> probeReadySemaphore;
        RefPtr<CommandPool> commandPool;
        RefPtr<DescriptorPool> descriptorPool;
        RefPtr<SceneV1::Scene> scene;
    };
}
