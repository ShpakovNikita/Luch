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
    struct SceneLoadContext
    {
        RefPtrVector<Mesh> loadedMeshes;
        RefPtrVector<Camera> loadedCameras;
        RefPtrVector<Buffer> loadedBuffers;
        RefPtrVector<PbrMaterial> loadedMaterials;
        RefPtrVector<Sampler> loadedSamplers;
        RefPtrVector<Texture> loadedTextures;
        RefPtrVector<Light> loadedLights;
    };

    class glTFLoader
    {
    public:
        glTFLoader(const String& rootFolder, SharedPtr<glTF::glTFRoot> glTFRoot);

        bool IsInterleaveEnabled() const { return interleave; }
        void SetInterleaveEnabled(bool aInterleave) { interleave = aInterleave; }

        int32 GetSceneCount() const;
        RefPtr<Scene> LoadScene(int32 index);
    private:
        SceneLoadContext LoadProperties();

        RefPtr<Scene> MakeScene(const glTF::Scene& scene, const SceneLoadContext& context);
        RefPtr<Node> MakeNode(const glTF::Node& node, const SceneLoadContext& context);
        RefPtr<Mesh> MakeMesh(const glTF::Mesh& mesh, const SceneLoadContext& context);
        RefPtr<Camera> MakeCamera(const glTF::Camera& camera, const SceneLoadContext& context);
        RefPtr<PerspectiveCamera> MakePerspectiveCamera(const String& name, const glTF::Perspective& camera);
        RefPtr<OrthographicCamera> MakeOrthographicCamera(const String& name, const glTF::Orthographic& camera);
        RefPtr<Primitive> MakePrimitiveInterleaved(const glTF::Primitive& primitive, const SceneLoadContext& context);
        RefPtr<Primitive> MakePrimitive(const glTF::Primitive& primitive, const SceneLoadContext& context);
        Optional<IndexBuffer> MakeIndexBuffer(const glTF::Accessor& indices, const SceneLoadContext& context);
        RefPtr<PbrMaterial> MakePbrMaterial(const glTF::Material& material, const SceneLoadContext& context);
        RefPtr<Texture> MakeTexture(const glTF::Texture& texture, const SceneLoadContext& context);
        RefPtr<Sampler> MakeSampler(const glTF::Sampler& sampler, const SceneLoadContext& context);
        RefPtr<Light> MakeLight(const glTF::LightPunctual& light, const SceneLoadContext& context);

        RefPtr<Buffer> ReadHostBuffer(const BufferSource& source);
        RefPtr<Image> ReadHostImage(const TextureSource& source);

        String rootFolder;
        SharedPtr<glTF::glTFRoot> root;

        bool interleave = true;
    };
}
