#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Camera.h>

namespace Husky::SceneV1
{
    Node::Node(
        RefPtrVector<Node>&& aChildren,
        const RefPtr<Mesh>& aMesh,
        const RefPtr<Camera>& aCamera,
        const TransformType &aTransform,
        const String &aName)
        : children(move(aChildren))
        , mesh(aMesh)
        , camera(aCamera)
        , transform(aTransform)
        , name(aName)
    {
    }

    Node::~Node() = default;
}
