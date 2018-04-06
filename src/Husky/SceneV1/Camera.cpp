#include <Husky/SceneV1/Camera.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Husky::SceneV1
{
    Camera::Camera(
        CameraType aType,
        const String &aName)
        : type(aType)
        , name(aName)
    {
    }

    PerspectiveCamera::PerspectiveCamera(
        float32 aYfov,
        float32 aZnear,
        Optional<float32> aZfar,
        Optional<float32> aAspectRatio,
        const String &aName)
        : Camera(CameraType::Perspective, aName)
        , yfov(aYfov)
        , znear(aZnear)
        , zfar(aZfar)
        , aspectRatio(aAspectRatio)
    {
    }

    OrthographicCamera::OrthographicCamera(
        float32 aXmag,
        float32 aYmag,
        float32 aZfar,
        float32 aZnear,
        const String& name)
        : Camera(CameraType::Orthographic, name)
        , xmag(aXmag)
        , ymag(aYmag)
        , zfar(aZfar)
        , znear(aZnear)
    {
    }
}
