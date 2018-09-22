#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>

namespace Husky::SceneV1
{
    Scene::Scene(
        RefPtrVector<Node> aNodes,
        SceneProperties aSceneProperties,
        const String &aName)
        : name(aName)
        , nodes(std::move(aNodes))
        , sceneProperties(std::move(aSceneProperties))
    {
    }

    Scene::~Scene() = default;

    void Scene::AddNode(const RefPtr<Node>& node)
    {
        nodes.push_back(node);

        // TODO scene properties
    }
}
