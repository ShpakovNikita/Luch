#include <Husky/SceneV1/Loader/glTFLoader.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/BufferSource.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Sampler.h>

namespace Husky::SceneV1::Loader
{
    glTFLoader::glTFLoader(const String& aRootFolder, SharedPtr<glTF::glTFRoot> glTFRoot)
        : rootFolder(aRootFolder)
        , root(move(glTFRoot))
    {
    }

    RefPtrVector<Scene> glTFLoader::LoadScenes()
    {
        if (!loaded)
        {
            
        }
        RefPtrVector<Scene> result;
        result.reserve(root->scenes.size());

        for (const auto& glTFScene : root->scenes)
        {
            
        }

        return result;
    }

    void glTFLoader::LoadProperties()
    {
        loadedNodes.reserve(root->nodes.size());
        for (const auto& node : root->nodes)
        {
            loadedNodes.emplace_back(MakeNode(node));
        }

        loadedMeshes.reserve(root->meshes.size());
        for (const auto& mesh: root->meshes)
        {
            loadedMeshes.emplace_back(MakeMesh(mesh));
        }

        loadedCameras.reserve(root->cameras.size());
        for (const auto& camera: root->cameras)
        {
            loadedCameras.emplace_back(MakeCamera(camera));
        }

        loadedBuffers.reserve(root->buffers.size());
        for (const auto& buffer : root->buffers)
        {
            BufferSource bufferSource;
            bufferSource.byteLength = buffer.byteLength;
            bufferSource.filename = buffer.uri;
            loadedBuffers.emplace_back(bufferSource);
        }
    }

    RefPtr<Scene> glTFLoader::MakeScene(const glTF::Scene& scene)
    {
        const String& name = scene.name;

        RefPtrVector<Node> nodes;
        nodes.reserve(scene.nodes.size());

        for (auto nodeIndex : scene.nodes)
        {
            nodes.emplace_back(loadedNodes[nodeIndex]);
        }

        return MakeRef<Scene>(move(nodes), name);
    }

    RefPtr<Node> glTFLoader::MakeNode(const glTF::Node& node)
    {
        const String& name = node.name;

        RefPtrVector<Mesh> meshes;
        meshes.reserve(node.children.size());

        for (auto meshIndex : node.children)
        {
            meshes.emplace_back(loadedMeshes[meshIndex]);
        }

        RefPtr<Camera> camera;
        if (node.camera.has_value())
        {
            auto cameraIndex = node.camera.value();
            camera = loadedCameras[cameraIndex];
        }

        // TODO skin

        Node::TransformType transform;

        if(node.matrix.has_value())
        {
            transform = node.matrix.value();
        }
        else
        {
            transform = Transform
            {
                node.rotation.value(),
                node.scale.value(),
                node.translation.value()
            };
        }

        return MakeRef<Node>(move(meshes), transform, name);
    }

    RefPtr<Mesh> glTFLoader::MakeMesh(const glTF::Mesh& mesh)
    {
        const auto& name = mesh.name;

        RefPtrVector<Primitive> primitives;

        for (const auto& primitive : mesh.primitives)
        {
            primitives.emplace_back(MakePrimitive(primitive));
        }

        return MakeRef<Mesh>(move(primitives), name);
    }

    RefPtr<Camera> glTFLoader::MakeCamera(const glTF::Camera& camera)
    {
        const auto& name = camera.name;

        switch (camera.type)
        {
        case glTF::CameraType::Orthographic:
            return MakeOrthographicCamera(name, camera.orthographic);
        case glTF::CameraType::Perspective:
            return MakePerspectiveCamera(name, camera.perspective);
        }
    }

    RefPtr<PerspectiveCamera> glTFLoader::MakePerspectiveCamera(const String& name, const glTF::Perspective& camera)
    {
        return MakeRef<PerspectiveCamera>(camera.yfov, camera.znear, camera.zfar, camera.aspectRatio, name);
    }

    RefPtr<OrthographicCamera> glTFLoader::MakeOrthographicCamera(const String& name, const glTF::Orthographic& camera)
    {
        return MakeRef<OrthographicCamera>(camera.xmag, camera.ymag, camera.zfar, camera.znear, name);
    }

    RefPtr<Primitive> glTFLoader::MakePrimitive(const glTF::Primitive& primitive)
    {
        Vector<PrimitiveAttribute> attributes;

        struct BufferWithIndex
        {
            RefPtr<VertexBuffer> buffer;
            int32 index;
        };

        Map<int32, BufferWithIndex> vertexBuffersMap;

        int32 currentVertexBufferIndex = 0;
        attributes.reserve(primitive.attributes.size());
        for (const auto& glTFAttribute : primitive.attributes)
        {
            const auto& accessor = root->accessors[glTFAttribute.accessor];

            PrimitiveAttribute attribute;
            attribute.attributeSemantic = (AttributeSemantic)glTFAttribute.semantic;
            attribute.componentType = (ComponentType)accessor.componentType;
            attribute.attributeType = (AttributeType)accessor.type;
            attribute.offset = accessor.byteOffset;

            const auto& bufferViewIndex = accessor.bufferView;

            RefPtr<VertexBuffer> vb;

            if (bufferViewIndex.has_value())
            {
                const auto& bufferView = root->bufferViews[bufferViewIndex.value()];

                int32 stride;
                if (bufferView.byteStride.has_value())
                {
                    stride = bufferView.byteStride.value();
                }
                else
                {
                    stride = CalculateStride(attribute.componentType, attribute.attributeType);
                }

                auto it = vertexBuffersMap.find(bufferViewIndex.value());
                if (it != vertexBuffersMap.end())
                {
                    vb = it->second.buffer;
                }
                else
                {
                    const auto& bufferSource = loadedBuffers[bufferView.buffer];
                    vb = MakeRef<VertexBuffer>(
                        bufferSource,
                        stride,
                        bufferView.byteOffset,
                        bufferView.byteLength);

                    vertexBuffersMap[bufferViewIndex.value()] = BufferWithIndex{ vb, currentVertexBufferIndex };

                    currentVertexBufferIndex++;
                }
            }

            attribute.vertexBufferIndex = currentVertexBufferIndex;
        }

        RefPtrVector<VertexBuffer> vertexBuffers;
        vertexBuffers.resize(vertexBuffersMap.size());
        for (const auto& kv : vertexBuffersMap)
        {
            vertexBuffers[kv.second.index] = kv.second.buffer;
        }

        RefPtr<IndexBuffer> indexBuffer;
        if (primitive.indices.has_value())
        {
            indexBuffer = MakeIndexBuffer(root->accessors[primitive.indices.value()]);
        }

        RefPtr<PbrMaterial> pbrMaterial;
        if (primitive.material.has_value())
        {
            pbrMaterial = loadedMaterials[primitive.material.value()];
        }

        return MakeRef<Primitive>(
            move(attributes),
            move(vertexBuffers),
            indexBuffer,
            pbrMaterial,
            PrimitiveTopology::TriangleList);
    }

    RefPtr<IndexBuffer> glTFLoader::MakeIndexBuffer(const glTF::Accessor& indices)
    {
        if (!indices.bufferView.has_value())
        {
            return RefPtr<IndexBuffer>();
        }

        const auto& bufferView = root->bufferViews[indices.bufferView.value()];
        HUSKY_ASSERT(!bufferView.byteStride.has_value(), "index accessor buffer views should not have stride");

        const auto& bufferSource = loadedBuffers[bufferView.buffer];

        IndexType indexType;

        switch (indices.componentType)
        {
        case glTF::ComponentType::UInt16:
            indexType = IndexType::UInt16;
            break;
        case glTF::ComponentType::UInt:
            indexType = IndexType::UInt32;
            break;
        default:
            HUSKY_ASSERT(false, "Unsupported index type");
        }

        HUSKY_ASSERT(indices.byteOffset == 0, "wtf");

        return MakeRef<IndexBuffer>(
            bufferSource,
            indexType,
            bufferView.byteOffset,
            bufferView.byteLength
            );
    }

    RefPtr<PbrMaterial> glTFLoader::MakePbrMaterial(const glTF::Material& glTFMaterial)
    {
        RefPtr<PbrMaterial> material = MakeRef<PbrMaterial>();

        material->name = glTFMaterial.name;

        return material;
    }

    RefPtr<Texture> glTFLoader::MakeTexture(const glTF::TextureInfo& textureInfo)
    {
        return RefPtr<Texture>();
    }

    RefPtr<Sampler> glTFLoader::MakeSampler(const glTF::Sampler & sampler)
    {
        return RefPtr<Sampler>();
    }

    constexpr int32 glTFLoader::CalculateStride(ComponentType componentType, AttributeType attributeType)
    {
        int32 componentSize = 0;
        int32 componentCount = 0;

        switch (componentType)
        {
        case ComponentType::Int8:
        case ComponentType::UInt8:
            componentSize = 1;
            break;
        case ComponentType::Int16:
        case ComponentType::UInt16:
            componentSize = 2;
            break;
        case ComponentType::UInt:
        case ComponentType::Float:
            componentSize = 4;
            break;
        }

        switch (attributeType)
        {
        case AttributeType::Scalar:
            componentCount = 1;
            break;
        case AttributeType::Vec2:
            componentCount = 2;
            break;
        case AttributeType::Vec3:
            componentCount = 3;
            break;
        case AttributeType::Vec4:
            componentCount = 4;
            break;
        case AttributeType::Mat2x2:
            componentCount = 4;
            break;
        case AttributeType::Mat3x3:
            componentCount = 9;
            break;
        case AttributeType::Mat4x4:
            componentCount = 16;
            break;
        }

        return componentSize * componentCount;
    }
}
