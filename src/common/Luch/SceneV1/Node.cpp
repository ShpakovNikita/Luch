#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Camera.h>

namespace Luch::SceneV1
{
    Node::~Node() = default;

    void Node::AddChild(const RefPtr<Node>& child)
    {
        children.push_back(child);
    }

    void Node::RemoveChild(Node* child)
    {
        // TODO
        // auto it = std::find(children.begin(), children.end(), child);
        // LUCH_ASSERT(it != children.end());
        // children.erase(it);
    }
}
