#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    struct SceneProperties
    {
        RefPtrSet<Mesh> meshes;
        RefPtrSet<Primitive> primitives;
        RefPtrSet<Camera> cameras;
        RefPtrSet<PbrMaterial> materials;
        RefPtrSet<Sampler> samplers;
        RefPtrSet<Texture> textures;
    };
}
