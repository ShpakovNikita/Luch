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
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Render/RenderContext.h>
#include <Luch/Render/Deferred/DeferredResources.h>
#include <Luch/Render/Deferred/DeferredOptions.h>
#include <Luch/Render/Deferred/DeferredShaderDefines.h>

#include <Luch/Render/SharedBuffer.h>

namespace Luch::Render
{
    using namespace Graphics;

    class SceneRenderer
    {
    public:
        SceneRenderer(
            const RefPtr<PhysicalDevice>& physicalDevice,
            const RefPtr<Surface>& surface,
            int32 width,
            int32 height);

        ~SceneRenderer();

        bool Initialize();
        bool Deinitialize();

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);
        void DrawScene(SceneV1::Scene* scene, SceneV1::Camera* camera);
    private:
        SharedPtr<RenderContext> context;

        int32 width = 0;
        int32 height = 0;
        Format swapchainFormat = Format::B8G8R8A8Unorm_sRGB;
    };
}
