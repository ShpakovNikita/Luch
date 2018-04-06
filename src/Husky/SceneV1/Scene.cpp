#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>

namespace Husky::SceneV1
{
    Scene::Scene(
        RefPtrVector<Node>&& aNodes,
        SceneProperties&& aSceneProperties,
        const String &aName)
        : nodes(move(aNodes))
        , sceneProperties(aSceneProperties)
        , name(aName)
    {
    }

    Scene::~Scene() = default;
}