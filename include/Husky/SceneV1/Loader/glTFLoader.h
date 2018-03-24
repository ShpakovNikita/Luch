#pragma once

#include <Husky/RefPtr.h>
#include <Husky/SharedPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/glTF2/Forwards.h>
#include <Husky/SceneV1/Primitive.h>

namespace Husky::SceneV1::Loader
{
    class glTFLoader
    {
    public:
        glTFLoader(const String& rootFolder, SharedPtr<glTF::glTFRoot> glTFRoot);

        RefPtrVector<Scene> LoadScenes();
    private:
        void LoadProperties();

        RefPtr<Scene> MakeScene(const glTF::Scene& scene);
        RefPtr<Node> MakeNode(const glTF::Node& node);
        RefPtr<Mesh> MakeMesh(const glTF::Mesh& mesh);
        RefPtr<Camera> MakeCamera(const glTF::Camera& camera);
        RefPtr<PerspectiveCamera> MakePerspectiveCamera(const String& name, const glTF::Perspective& camera);
        RefPtr<OrthographicCamera> MakeOrthographicCamera(const String& name, const glTF::Orthographic& camera);
        RefPtr<Primitive> MakePrimitive(const glTF::Primitive& primitive);
        RefPtr<IndexBuffer> MakeIndexBuffer(const glTF::Accessor& indices);
        RefPtr<VertexBuffer> MakeVertexBuffer(const glTF::Attribute& indices);
        RefPtr<PbrMaterial> MakePbrMaterial(const glTF::Material& material);
        RefPtr<Texture> MakeTexture(const glTF::TextureInfo& textureInfo);

        constexpr int32 CalculateStride(ComponentType componentType, AttributeType attributeType);

        String rootFolder;
        SharedPtr<glTF::glTFRoot> root;

        bool loaded = false;
        Vector<RefPtr<Node>> loadedNodes;
        Vector<RefPtr<Mesh>> loadedMeshes;
        Vector<RefPtr<Camera>> loadedCameras;
        Vector<BufferSource> loadedBuffers;

        Set<int32> buffersWithIndices;
        Set<int32> buffersWithVertices;
        Vector<RefPtr<Buffer>> buffers;

    };
}
