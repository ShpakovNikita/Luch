#include <Luch/MouseNodeController.h>
#include <Luch/VectorTypes.h>
#include <Luch/SceneV1/Node.h>

namespace Luch
{
    void MouseNodeController::Add(float32 dx, float32 dy)
    {
        ds[0] += dx;
        ds[1] += dy;
    }

    void MouseNodeController::Tick()
    {
        if(node == nullptr)
        {
            return;
        }

        auto transform = node->GetLocalTransform();

        if(std::holds_alternative<Mat4x4>(transform))
        {
            auto mat = std::get<Mat4x4>(transform);

            Mat4x4 pitchMat = glm::rotate(ds[1] * sensitivity[1], Vec3 {1, 0, 0});
            Mat4x4 yawMat = glm::rotate(ds[0] * sensitivity[0], Vec3 {0, 1, 0});

            transform = yawMat * mat * pitchMat;
        }
        else if(std::holds_alternative<SceneV1::TransformProperties>(transform))
        {
            auto props = std::get<SceneV1::TransformProperties>(transform);

            Vec3 pitchAxis{1, 0, 0};
            Quaternion pitchQuat = glm::angleAxis(ds[1] * sensitivity[1], pitchAxis);

            Vec3 yawAxis{0, 1, 0};
            Quaternion yawQuat = glm::angleAxis(ds[0] * sensitivity[0], yawAxis);

            props.rotation = yawQuat * props.rotation * pitchQuat;

            transform = props;
        }
        else
        {
            LUCH_ASSERT(false);
        }

        ds[0] = ds[1] = 0.0f;
        node->SetLocalTransform(transform);
    }
}
