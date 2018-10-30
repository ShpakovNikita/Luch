#include <Husky/SceneV1/Camera.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Husky::SceneV1
{
    Camera::Camera(CameraType aType)
        : type(aType)
    {
    }

    Mat4x4 Camera::GetCameraViewMatrix() const
    {
        return viewMatrix;
    }

    void Camera::SetCameraViewMatrix(const Mat4x4& view)
    {
        viewMatrix = view;
    }

    Vec3 Camera::GetCameraPosition()
    {
        return viewMatrix[3];
    }

    PerspectiveCamera::PerspectiveCamera()
        : Camera(CameraType::Perspective)
    {
    }

    PerspectiveCamera::PerspectiveCamera(
        float32 aYfov,
        float32 aZnear,
        Optional<float32> aZfar,
        Optional<float32> aAspectRatio)
        : Camera(CameraType::Perspective)
        , yfov(aYfov)
        , znear(aZnear)
        , zfar(aZfar)
        , aspectRatio(aAspectRatio)
    {
    }

    Mat4x4 PerspectiveCamera::GetCameraProjectionMatrix() const
    {
        float32 aspect = aspectRatio.value_or(viewAspectRatio);

        if (zfar.has_value())
        {
            return glm::perspective(yfov, aspect, znear, *zfar);
        }
        else
        {
            return glm::infinitePerspective(yfov, aspect, znear);
        }

        //// https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#projection-matrices        
        //
        //float32 y = yfov;
        //float32 n = znear;
        //
        //float32 rt = 1/std::tanf(0.5 * y);
        //
        //if (zfar.has_value())
        //{
        //    // Finite
        //    float32 f = zfar.value();
        //    float32 rnf = 1/(n-f);
        //
        //    return Mat4x4
        //    {
        //        rt / a, 0, 0, 0,
        //        0, rt, 0, 0,
        //        0, 0, (f+n)*rnf, 2*f*n*rnf,
        //        0, 0, -1, 0
        //    };
        //}
        //else
        //{
        //    // Infinite
        //    return Mat4x4
        //    {
        //        rt / a, 0, 0, 0,
        //        0, rt, 0, 0,
        //        0, 0, -1, -2 * n,
        //        0, 0, -1, 0
        //    };
        //}
        
    }

    OrthographicCamera::OrthographicCamera()
        : Camera(CameraType::Orthographic)
    {
    }

    OrthographicCamera::OrthographicCamera(
        float32 aXmag,
        float32 aYmag,
        float32 aZfar,
        float32 aZnear)
        : Camera(CameraType::Orthographic)
        , xmag(aXmag)
        , ymag(aYmag)
        , zfar(aZfar)
        , znear(aZnear)
    {
    }

    Mat4x4 OrthographicCamera::GetCameraProjectionMatrix() const
    {
        float32 r = xmag;
        float32 t = ymag;
        float32 f = zfar;
        float32 n = znear;

        float32 rnf = 1 / (n - f);

        return Mat4x4
        {
            1/r, 0, 0, 0,
            0, 1/t, 0, 0,
            0, 0, 2*rnf, (f+n)*rnf,
            0, 0, 0, 1
        };
    }
}
