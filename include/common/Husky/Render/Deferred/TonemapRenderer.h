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
#include <Husky/Render/Common.h>
#include <Husky/Render/RenderForwards.h>
#include <Husky/Render/Deferred/DeferredForwards.h>

namespace Husky::Render::Deferred
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
