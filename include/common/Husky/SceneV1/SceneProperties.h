#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SceneV1/SceneV1Forwards.h>

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
        RefPtrUnorderedSet<Light> lights;
        RefPtrUnorderedSet<Node> lightNodes;
    };
}
