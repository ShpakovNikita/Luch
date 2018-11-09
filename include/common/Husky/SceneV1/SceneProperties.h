#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SceneV1/SceneV1Forwards.h>

namespace Husky::SceneV1
{
    struct SceneProperties
    {
        template<typename T>
        using Usages = UnorderedSet<RefPtr<T>, RefPtrHash<T>>;

        Usages<Buffer> buffers;
        Usages<Mesh> meshes;
        Usages<Primitive> primitives;
        Usages<Camera> cameras;
        Usages<Node> cameraNodes;
        Usages<PbrMaterial> materials;
        Usages<Texture> textures;
        Usages<Light> lights;
        Usages<Node> lightNodes;
    };
}
