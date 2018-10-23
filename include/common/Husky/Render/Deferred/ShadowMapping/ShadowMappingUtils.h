#pragma once

#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    // 1 (for directional and spot) or 6 (for point light)
    int32 GetLightCamerasCount(const SceneV1::Light* light);

    Mat4x4 GetSpotLightProjectionMatrix(const SceneV1::Light* light, float32 znear);
    // TODO
    //Mat4x4 GetDirectionalLightProjectionMatrix(const SceneV1::Light* light);

    Mat4x4 GetPointLightProjectionMatrix(const SceneV1::Light* light, float32 znear);
}
