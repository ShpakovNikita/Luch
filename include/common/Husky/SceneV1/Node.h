#pragma once

#include <Husky/BaseObject.h>
#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/SceneV1/SceneV1Forwards.h>

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

        Node() = default;
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

        inline const TransformType& GetLocalTransform() const { return localTransform; }
        void SetLocalTransform(const TransformType& aLocalTransform) { localTransform = aLocalTransform; }

        Mat4x4 GetWorldTransform() const { return worldTransform; }
        void SetWorldTransform(const Mat4x4 aWorldTransform)  { worldTransform = aWorldTransform; }
    private:
        String name;
        Node *parent = nullptr;
        RefPtrVector<Node> children;

        RefPtr<Mesh> mesh;
        RefPtr<Camera> camera;
        RefPtr<Light> light;

        TransformType localTransform = Mat4x4{ 1.0 };
        Mat4x4 worldTransform = Mat4x4 { 1.0 };
    };
}
