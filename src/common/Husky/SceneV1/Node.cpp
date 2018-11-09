#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Camera.h>

namespace Husky::SceneV1
{
    Node::~Node() = default;

    void Node::AddChild(const RefPtr<Node>& child)
    {
        
    }

    void Node::RemoveChild(Node* child)
    {

    }
}
