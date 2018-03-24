#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    class Mesh : public BaseObject
    {
    public:
        Mesh(
            const RefPtrVector<Primitive>& primitives,
            const String& name = "");
    private:
        String name;
        RefPtrVector<Primitive> primitives;
    };
}
