#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/Render/Common.h>
#include <Husky/ResultValue.h>

namespace Husky::Render
{
    using namespace Graphics;

    namespace RenderUtils
    {
        MaterialUniform GetMaterialUniform(SceneV1::PbrMaterial *material);
        CameraUniform GetCameraUniform(SceneV1::Camera *camera, const Mat4x4& transform);
        LightUniform GetLightUniform(SceneV1::Light* light, const Mat4x4& worldTransform);

        ResultValue<bool, RefPtr<ShaderLibrary>> CreateShaderLibrary(
            GraphicsDevice* device,
            const String& path,
            const UnorderedMap<String, Variant<int32, String>>& defines);
    }
}
