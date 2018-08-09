#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    struct SceneProperties
    {
        RefPtrUnorderedSet<Buffer> buffers;
        RefPtrUnorderedSet<Mesh> meshes;
        RefPtrUnorderedSet<Primitive> primitives;
        RefPtrUnorderedSet<Camera> cameras;
        RefPtrUnorderedSet<PbrMaterial> materials;
        RefPtrUnorderedSet<Texture> textures;
        RefPtrUnorderedSet<Sampler> samplers;
    };
}
