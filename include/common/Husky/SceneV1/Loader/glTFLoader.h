#pragma once

#include <Husky/VectorTypes.h>
#include <Husky/RefPtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/glTF2/glTFForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/BufferSource.h>
#include <Husky/SceneV1/Primitive.h>

namespace Husky::SceneV1::Loader
{
    #pragma pack(push, 1)
    struct InterleavedVertex
    {
        Vec3 position;
        Vec3 normal;
        Vec4 tangent;
        Vec2 texcoord;
    };
    #pragma pack(pop)

    class glTFLoader
    {
    public:
        glTFLoader(const String& rootFolder, SharedPtr<glTF::glTFRoot> glTFRoot);

        bool IsInterleaveEnabled() const { return interleave; }
        void SetInterleaveEnabled(bool aInterleave) { interleave = aInterleave; }
        RefPtrVector<Scene> LoadScenes();
    private:
        void LoadProperties();

        RefPtr<Scene> MakeScene(const glTF::Scene& scene);
        RefPtr<Node> MakeNode(const glTF::Node& node, SceneProperties* context);
        RefPtr<Mesh> MakeMesh(const glTF::Mesh& mesh);
        RefPtr<Camera> MakeCamera(const glTF::Camera& camera);
        RefPtr<PerspectiveCamera> MakePerspectiveCamera(const String& name, const glTF::Perspective& camera);
        RefPtr<OrthographicCamera> MakeOrthographicCamera(const String& name, const glTF::Orthographic& camera);
        RefPtr<Primitive> MakePrimitiveInterleaved(const glTF::Primitive& primitive);
        RefPtr<Primitive> MakePrimitive(const glTF::Primitive& primitive);
        Optional<IndexBuffer> MakeIndexBuffer(const glTF::Accessor& indices);
        RefPtr<PbrMaterial> MakePbrMaterial(const glTF::Material& material);
        RefPtr<Texture> MakeTexture(const glTF::Texture& texture);
        RefPtr<Sampler> MakeSampler(const glTF::Sampler& sampler);

        RefPtr<Buffer> ReadHostBuffer(const BufferSource& source);
        RefPtr<Image> ReadHostImage(const TextureSource& source);

        String rootFolder;
        SharedPtr<glTF::glTFRoot> root;

        bool loaded = false;
        bool interleave = true;
        RefPtrVector<Mesh> loadedMeshes;
        RefPtrVector<Camera> loadedCameras;
        RefPtrVector<Buffer> loadedBuffers;
        RefPtrVector<PbrMaterial> loadedMaterials;
        RefPtrVector<Sampler> loadedSamplers;
        RefPtrVector<Texture> loadedTextures;
    };
}
