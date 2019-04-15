#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/SharedPtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/SharedBuffer.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Graph/RenderGraphPassAttachmentConfig.h>
#include <Luch/Render/Passes/IBL/IBLForwards.h>
#include <Luch/Render/Techniques/Forward/ForwardForwards.h>

namespace Luch::Render::Passes::IBL
{
    using namespace Graphics;
    using namespace Graph;

    class EnvironmentCubemapRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
    public:
        static constexpr Format LuminanceFormat = Format::RGBA16Sfloat;
        static const String RenderPassName;

        static ResultValue<bool, UniquePtr<EnvironmentCubemapPersistentContext>> PrepareEnvironmentCubemapPersistentContext(
            const EnvironmentCubemapPersistentContextCreateInfo& createInfo);

        static ResultValue<bool, UniquePtr<EnvironmentCubemapTransientContext>> PrepareEnvironmentCubemapTransientContext(
            EnvironmentCubemapPersistentContext* persistentContext,
            const EnvironmentCubemapTransientContextCreateInfo& createInfo);

        EnvironmentCubemapRenderPass(
            EnvironmentCubemapPersistentContext* persistentContext,
            EnvironmentCubemapTransientContext* transientContext);

        ~EnvironmentCubemapRenderPass();

        void PrepareScene();
        void UpdateScene();

        inline RenderGraphPassAttachmentConfig& GetMutableAttachmentConfig() { return attachmentConfig; }

        inline RenderMutableResource GetEnvironmentLuminanceCubemapHandle() { return luminanceCubemapHandle; }
        inline RenderMutableResource GetEnvironmentDepthCubemapHandle() { return depthCubemapHandle; }

        void Initialize(RenderGraphBuilder* builder) override;

        void ExecuteGraphicsPass(
            RenderGraphResourceManager* manager,
            GraphicsCommandList* commandList) override;
    private:
        RenderGraphPassAttachmentConfig attachmentConfig;
        UniquePtr<Techniques::Forward::ForwardRenderer> renderer;

        int32 face = 0;

        EnvironmentCubemapPersistentContext* persistentContext = nullptr;
        EnvironmentCubemapTransientContext* transientContext = nullptr;

        RenderMutableResource luminanceCubemapHandle;
        RenderMutableResource depthCubemapHandle;
    };
}
