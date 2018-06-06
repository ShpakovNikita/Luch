#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    struct SceneProperties
    {
        template<typename T>
        using PropertiesMap = UnorderedMap<ID<T>, RefPtr<T>>;

        RefPtrUnorderedSet<Mesh> meshes;
        RefPtrUnorderedSet<Primitive> primitives;
        RefPtrUnorderedSet<Camera> cameras;
        RefPtrUnorderedSet<PbrMaterial> materials;
        RefPtrUnorderedSet<Sampler> samplers;
        RefPtrUnorderedSet<Texture> textures;
    };
}
