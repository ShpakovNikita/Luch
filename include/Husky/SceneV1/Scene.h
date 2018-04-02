#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    class Scene : public BaseObject
    {
    public:
        Scene(
            RefPtrVector<Node>&& nodes,
            const String& name);

        ~Scene();
    private:
        String name;
        RefPtrVector<Node> nodes;
    };
}
