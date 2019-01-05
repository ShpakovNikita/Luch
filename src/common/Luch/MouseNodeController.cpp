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

            mat *= glm::rotate(ds[0] * speed[0], Vec3 {0, 1, 0});
            mat *= glm::rotate(ds[1]* speed[1], Vec3 {1, 0, 0});

            transform = mat;
        }
        else if(std::holds_alternative<SceneV1::TransformProperties>(transform))
        {
            auto props = std::get<SceneV1::TransformProperties>(transform);

            props.rotation = glm::rotate(props.rotation, ds[0] * speed[0], Vec3 {0, 1, 0});
            props.rotation = glm::rotate(props.rotation, ds[1] * speed[1], Vec3 {1, 0, 0});

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
