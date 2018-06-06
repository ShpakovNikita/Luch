#pragma once

#include <Husky/ID.h>

namespace Husky::SceneV1
{
    class Buffer;
    class Camera;
    class Image;
    class IndexBuffer;
    class Light;
    class Mesh;
    class Node;
    class OrthographicCamera;
    class PbrMaterial;
    class PerspectiveCamera;
    class Primitive;
    class Sampler;
    class Scene;
    class SceneProperties;
    class Texture;
    class TextureInfo;
    class VertexBuffer;

    struct BufferSource;
    struct TextureSource;

    using MeshID = ID<Mesh>;
    using PrimitiveID = ID<Primitive>;
    using CameraID = ID<Camera>;
    using PbrMaterialID = ID<PbrMaterial>;
    using SamplerID = ID<Sampler>;
    using TextureID = ID<Texture>;

}
