#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Light.h>

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

        const auto& mesh = node->GetMesh();
        if(mesh != nullptr)
        {
            sceneProperties.meshes.insert(mesh);
        }

        const auto& light = node->GetLight();
        if(light != nullptr)
        {
            sceneProperties.lightNodes.insert(node);
            sceneProperties.lights.insert(light);
        }
    }
}
