#pragma once

#include <Husky/BaseObject.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    struct TransformProperties
    {
        Quaternion rotation;
        Vec3 scale = { 1.0f, 1.0f, 1.0f };
        Vec3 translation = { 0, 0, 0 };
    };

    class Node : public BaseObject
    {
    public:
        using TransformType = Variant<Mat4x4, TransformProperties>;

        /*Node(
            const RefPtrVector<Mesh>& children,
            const Mat4x4 matrix,
            const String& name);

        Node(
            const RefPtrVector<Mesh>& children,
            const Transform& transform,
            const String& name);*/

        Node(
            RefPtrVector<Node>&& children,
            const RefPtr<Mesh>& mesh,
            const RefPtr<Camera>& camera,
            const TransformType& transform,
            const String& name);

        ~Node();

        inline const String& GetName() const { return name; }
        inline const RefPtrVector<Node>& GetChildren() const { return children; }
        inline const RefPtr<Mesh>& GetMesh() const { return mesh; }
        inline const RefPtr<Camera>& GetCamera() const { return camera; }
        inline const TransformType& GetTransform() const { return transform; }
    private:
        String name;
        RefPtrVector<Node> children;
        RefPtr<Mesh> mesh;
        RefPtr<Camera> camera;
        TransformType transform;
    };
}
