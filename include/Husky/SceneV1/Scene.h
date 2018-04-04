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
        Scene(
            RefPtrVector<Node>&& nodes,
            const String& name);

        ~Scene();

        inline const String& GetName() const { return name;  }
        inline const SceneProperties& GetSceneProperties() const { return sceneProperties; }
    private:
        String name;
        SceneProperties sceneProperties;
    };
}
