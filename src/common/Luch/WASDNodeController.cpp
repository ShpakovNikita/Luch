#include <Luch/WASDNodeController.h>
#include <Luch/VectorTypes.h>
#include <Luch/SceneV1/Node.h>

namespace Luch
{
    void WASDNodeController::Tick(float32 dt)
    {
        if(dt <= 0.0f)
        {
            return;
        }

        if(node == nullptr)
        {
            return;
        }

        Vec3 v {};

        for(int32 axis = XAxis; axis <= ZAxis; axis++)
        {
            if(moving[axis][Positive] == moving[axis][Negative])
            {
                continue;
            }
            else if(moving[axis][Positive])
            {
                v[axis] = speed[axis][Positive];
            }
            else if(moving[axis][Negative])
            {
                v[axis] = -speed[axis][Negative];
            }
        }

        Vec3 vSwizzled = { v.y, v.z, v.x };

        Vec3 translation = vSwizzled * dt;

        auto transform = node->GetLocalTransform();

        if(std::holds_alternative<Mat4x4>(transform))
        {
            auto mat = std::get<Mat4x4>(transform);
            Mat3x3 view;
            view[0] = { mat[0].x, mat[1].y, mat[1].z };
            view[1] = { mat[1].x, mat[1].y, mat[1].z };
            view[2] = { mat[2].x, mat[2].y, mat[2].z };
            Vec3 viewTranslation = view * translation;
            transform =  glm::translate(mat, viewTranslation);
        }
        else if(std::holds_alternative<SceneV1::TransformProperties>(transform))
        {
            auto props = std::get<SceneV1::TransformProperties>(transform);
            Mat4x4 view = glm::toMat4(props.rotation) * glm::scale(props.scale);
            Vec4 viewTranslation = view * Vec4 { translation, 0.0 }; 
            props.translation += Vec3 { viewTranslation };
            transform = props;
        }
        else
        {
            LUCH_ASSERT(false);
        }

        node->SetLocalTransform(transform);
    }
}
