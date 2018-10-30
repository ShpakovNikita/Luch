#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingUtils.h>
#include <Husky/SceneV1/Light.h>
#include <Husky/Assert.h>

namespace Husky::Render::Deferred::ShadowMapping
{
    int32 GetLightCamerasCount(const SceneV1::Light* light)
    {
        switch(light->GetType())
        {
        case SceneV1::LightType::Point:
            return 6;
        case SceneV1::LightType::Spot:
        case SceneV1::LightType::Directional:
            return 1;
        default:
            HUSKY_ASSERT(false);
            return 0;
        }
    }

    Mat4x4 GetSpotLightProjectionMatrix(const SceneV1::Light* light, float32 znear)
    {
        HUSKY_ASSERT(light->GetSpotlightAngle().has_value());
        HUSKY_ASSERT(light->GetRange().has_value());

        float32 yfov = light->GetSpotlightAngle().value();
        float32 aspectRatio = 1;
        float32 zfar = light->GetRange().value();

        return glm::perspective(yfov, aspectRatio, znear, zfar);
    }

    // TODO
    // Mat4x4 GetDirectionalLightProjectionMatrix(const SceneV1::Light* light, )
    // {
    // }

    Mat4x4 GetPointLightProjectionMatrix(const SceneV1::Light* light, float32 znear)
    {
        HUSKY_ASSERT(light->GetRange().has_value());

        float32 yfov = glm::pi<float32>() / 2;
        float32 aspectRatio = 1;
        float32 zfar = light->GetRange().value();

        return glm::perspective(yfov, aspectRatio, znear, zfar);
    }
}
