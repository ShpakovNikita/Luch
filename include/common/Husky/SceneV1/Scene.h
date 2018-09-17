#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>
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

        inline const RefPtr<Light>& GetAmbientLight() const { return ambientLight; }
        inline void SetAmbientLight(const RefPtr<Light>& aAmbientLight) { ambientLight = aAmbientLight; }

        void AddNode(const RefPtr<Node>& node);
    private:
        String name;
        RefPtrVector<Node> nodes;
        RefPtr<Light> ambientLight;
        SceneProperties sceneProperties;
    };
}
