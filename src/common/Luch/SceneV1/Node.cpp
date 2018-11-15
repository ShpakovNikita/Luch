#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Camera.h>

namespace Luch::SceneV1
{
    Node::~Node() = default;

    void Node::AddChild(const RefPtr<Node>& child)
    {
        
    }

    void Node::RemoveChild(Node* child)
    {

    }
}
