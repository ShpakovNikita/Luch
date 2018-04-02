#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Node.h>

namespace Husky::SceneV1
{
    Scene::Scene(
        RefPtrVector<Node>&& aNodes,
        const String &aName)
        : nodes(move(aNodes))
        , name(aName)
    {
    }

    Scene::~Scene() = default;
}