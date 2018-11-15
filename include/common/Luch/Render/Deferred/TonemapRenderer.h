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
#include <Luch/Render/Common.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    class TonemapRenderer
    {
    public:
        static constexpr int32 SharedUniformBufferSize = 256 * 1024;
        static const String RendererName;

        TonemapRenderer();
        ~TonemapRenderer();

        const SharedPtr<RenderContext>& GetRenderContext() { return context; }
        void SetRenderContext(const SharedPtr<RenderContext>& aContext) { context = aContext; }

        bool Initialize();
        bool Deinitialize();

        void Tonemap(Texture* input, Texture* output);
    private:
        static RefPtr<PipelineState> CreateTonemapPipelineState(RenderContext* context, TonemapPassResources* resources);
        static ResultValue<bool, UniquePtr<TonemapPassResources>> PrepareTonemapPassResources(RenderContext* context);

        SharedPtr<RenderContext> context;
        UniquePtr<TonemapPassResources> resources;
    };
}
