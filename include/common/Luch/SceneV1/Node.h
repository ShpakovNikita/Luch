#pragma once

#include <Luch/BaseObject.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::SceneV1
{
    struct TransformProperties
    {
        Quaternion rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
        Vec3 scale = { 1.0f, 1.0f, 1.0f };
        Vec3 translation = { 0.0f, 0.0f, 0.0f };
    };

    class Node : public BaseObject
    {
        friend class Scene;
    public:
        using TransformType = Variant<Mat4x4, TransformProperties>;

        Node() = default;
        ~Node();

        inline const String& GetName() const { return name; }
        inline void SetName(const String& aName) { name = aName; }

        inline Node* GetParent() const { return parent; }
        inline bool HasParent() const { return parent != nullptr; }

        inline const RefPtrVector<Node>& GetChildren() const { return children; }

        inline const RefPtr<Mesh>& GetMesh() const { return mesh; }
        void SetMesh(const RefPtr<Mesh>& aMesh) { mesh = aMesh; }

        inline const RefPtr<Camera>& GetCamera() const { return camera; }
        void SetCamera(const RefPtr<Camera>& aCamera) { camera = aCamera; }

        inline const RefPtr<Light>& GetLight() const { return light; }
        void SetLight(const RefPtr<Light>& aLight) { light = aLight; }

        inline const RefPtr<LightProbe>& GetLightProbe() const { return lightProbe; }
        void SetLightProbe(const RefPtr<LightProbe>& aLightProbe) { lightProbe = aLightProbe; }

        inline const TransformType& GetLocalTransform() const { return localTransform; }
        void SetLocalTransform(const TransformType& aLocalTransform) { localTransform = aLocalTransform; }

        Mat4x4 GetWorldTransform() const { return worldTransform; }
        void SetWorldTransform(const Mat4x4 aWorldTransform)  { worldTransform = aWorldTransform; }

        void AddChild(const RefPtr<Node>& child);
        void RemoveChild(Node* child);

    private:
        String name;
        Node* parent = nullptr;
        Scene* scene = nullptr;
        RefPtrVector<Node> children;

        RefPtr<Mesh> mesh;
        RefPtr<Camera> camera;
        RefPtr<Light> light;
        RefPtr<LightProbe> lightProbe;

        TransformType localTransform = Mat4x4{ 1.0 };
        Mat4x4 worldTransform = Mat4x4 { 1.0 };
    };
}
