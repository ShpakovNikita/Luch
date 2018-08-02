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
        Quaternion rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
        Vec3 scale = { 1.0f, 1.0f, 1.0f };
        Vec3 translation = { 0.0f, 0.0f, 0.0f };
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

        Node() = default;

        Node(
            RefPtrVector<Node> children,
            const RefPtr<Mesh>& mesh,
            const RefPtr<Camera>& camera,
            const TransformType& transform,
            const String& name);

        ~Node();

        inline const String& GetName() const { return name; }
        inline void SetName(const String& aName) { name = aName; }

        inline Node* GetParent() const { return parent; }
        inline bool HasParent() const { return parent != nullptr; }
        inline void SetParent(Node *aParent) { parent = aParent; }

        inline const RefPtrVector<Node>& GetChildren() const { return children; }

        inline const RefPtr<Mesh>& GetMesh() const { return mesh; }
        void SetMesh(const RefPtr<Mesh>& aMesh);

        inline const RefPtr<Camera>& GetCamera() const { return camera; }
        void SetCamera(const RefPtr<Camera>& aCamera);

        inline const RefPtr<Light>& GetLight() const { return light; }
        void SetLight(const RefPtr<Light>& aLight);

        inline const TransformType& GetTransform() const { return transform; }
        void SetTransform(const TransformProperties& transform);
        void SetTransform(const Mat4x4& matrix);
    private:
        String name;
        Node *parent = nullptr;
        RefPtrVector<Node> children;

        RefPtr<Mesh> mesh;
        RefPtr<Camera> camera;
        RefPtr<Light> light;

        TransformType transform = Mat4x4{1.0};
    };
}
