#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Camera.h>

namespace Husky::SceneV1
{
    Node::Node(
        RefPtrVector<Node> aChildren,
        const RefPtr<Mesh>& aMesh,
        const RefPtr<Camera>& aCamera,
        const TransformType &aTransform,
        const String &aName)
        : name(aName)
        , children(move(aChildren))
        , mesh(aMesh)
        , camera(aCamera)
        , transform(aTransform)
    {
    }

    Node::~Node() = default;

    void Node::SetMesh(const RefPtr<Mesh>& aMesh)
    {
        mesh = aMesh;
        camera = nullptr;
        light = nullptr;
    }

    void Node::SetCamera(const RefPtr<Camera>& aCamera)
    {
        camera = aCamera;
        light = nullptr;
        mesh = nullptr;
    }

    void Node::SetLight(const RefPtr<Light>& aLight)
    {
        light = aLight;
        mesh = nullptr;
        camera = nullptr;
    }

    void Node::SetTransform(const TransformProperties& aTransform)
    {
        transform = aTransform;
    }

    void Node::SetTransform(const Mat4x4& matrix)
    {
        transform = matrix;
    }
}
