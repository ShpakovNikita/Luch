#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/ResultValue.h>
#include <functional>

namespace Luch::Render
{
    using namespace Graphics;

    namespace RenderUtils
    {
        MaterialUniform GetMaterialUniform(SceneV1::PbrMaterial *material);
        CameraUniform GetCameraUniform(SceneV1::Camera *camera, const Mat4x4& transform);
        LightUniform GetLightUniform(SceneV1::Light* light, const Mat4x4& worldTransform);

        ResultValue<bool, RefPtr<ShaderLibrary>> CreateShaderLibrary(
            GraphicsDevice* device,
            const String& includeDir,
            const String& dir,
            const String& filename,
            const UnorderedMap<String, Variant<int32, String>>& defines);

        ResultValue<bool, String> SubstituteIncludes(
            const String& includeDir,
            String source);

        void SubmitCommandLists(
            const RefPtr<CommandQueue>& queue,
            const RefPtrVector<CommandList>& commandLists,
            const std::function<void()> completedHandler = {});
    }
}
