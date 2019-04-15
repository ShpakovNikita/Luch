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
#include <Luch/Render/Passes/Forward/ForwardForwards.h>
#include <Luch/Render/Techniques/Forward/ForwardForwards.h>

namespace Luch::Render::Passes::Forward
{
    using namespace Graphics;
    using namespace Graph;

    class ForwardRenderPass : public RenderGraphPass
    {
        static constexpr int32 MaxDescriptorSetCount = 4096;
        static constexpr int32 MaxDescriptorCount = 4096;
        static constexpr Format LuminanceFormat = Format::RGBA16Sfloat;
    public:
        static const String RenderPassName;
        static const String RenderPassNameWithDepthPrepass;

        static ResultValue<bool, UniquePtr<ForwardPersistentContext>> PrepareForwardPersistentContext(
            const ForwardPersistentContextCreateInfo& createInfo);

        static ResultValue<bool, UniquePtr<ForwardTransientContext>> PrepareForwardTransientContext(
            ForwardPersistentContext* persistentContext,
            const ForwardTransientContextCreateInfo& createInfo);

        ForwardRenderPass(
            ForwardPersistentContext* persistentContext,
            ForwardTransientContext* transientContext);

        ~ForwardRenderPass();

        void PrepareScene();
        void UpdateScene();

        inline RenderGraphPassAttachmentConfig& GetMutableAttachmentConfig() { return attachmentConfig; }
        inline RenderMutableResource GetLuminanceTextureHandle() { return luminanceTextureHandle; }

        void Initialize(RenderGraphBuilder* builder) override;

        void ExecuteGraphicsPass(
            RenderGraphResourceManager* manager,
            GraphicsCommandList* commandList) override;
    private:
        static const String& GetRenderPassName(bool useDepthPrepass);

        RenderGraphPassAttachmentConfig attachmentConfig;

        ForwardPersistentContext* persistentContext = nullptr;
        ForwardTransientContext* transientContext = nullptr;

        UniquePtr<Techniques::Forward::ForwardRenderer> renderer;

        RenderMutableResource luminanceTextureHandle;
        RenderMutableResource depthStencilTextureHandle;

        RenderResource diffuseIlluminanceCubemapHandle;
        RenderResource specularReflectionCubemapHandle;
        RenderResource specularBRDFTextureHandle;
    };
}
