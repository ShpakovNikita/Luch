#pragma once

#include <Husky/BaseObject.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    struct Transform
    {
        Quaternion rotation;
        Vec3 scale = { 1.0f, 1.0f, 1.0f };
        Vec3 translation = { 0, 0, 0 };
    };

    class Node : public BaseObject
    {
    public:
        using TransformType = Variant<Mat4x4, Transform>;

        Node(
            const RefPtrVector<Mesh>& children,
            const Mat4x4 matrix,
            const String& name);

        Node(
            const RefPtrVector<Mesh>& children,
            const Transform& transform,
            const String& name);

        Node(
            const RefPtrVector<Mesh>& children,
            const TransformType& transform,
            const String& name);
    private:
        String name;
        RefPtrVector<Mesh> children;
        TransformType transform;
    };
}
