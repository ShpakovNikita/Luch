#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/SceneProperties.h>

namespace Husky::SceneV1
{
    class Scene : public BaseObject
    {
    public:
        Scene();

        Scene(
            RefPtrVector<Node> nodes,
            SceneProperties sceneProperties,
            const String& name);

        ~Scene();

        inline const String& GetName() const { return name; }
        inline void SetName(const String& aName) { name = aName; }

        inline const SceneProperties& GetSceneProperties() const { return sceneProperties; }
        inline const RefPtrVector<Node>& GetNodes() const { return nodes; }

        void AddNode(const RefPtr<Node>& node);
    private:
        String name;
        RefPtrVector<Node> nodes;
        SceneProperties sceneProperties;
    };
}
