#include <Husky/SceneV1/Loader/glTFLoader.h>
#include <Husky/SceneV1/Loader/LoaderUtils.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Vulkan.h>
#include <Husky/UniquePtr.h>
#include <Husky/FileStream.h>
#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Node.h>
#include <Husky/SceneV1/Image.h>
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
            LoadProperties();
            loaded = true;
        }

        RefPtrVector<Scene> result;
        result.reserve(root->scenes.size());

        for (const auto& scene : root->scenes)
        {
            result.emplace_back(MakeScene(scene));
        }

        return result;
    }

    void glTFLoader::LoadProperties()
    {
        loadedCameras.reserve(root->cameras.size());
        for (const auto& camera : root->cameras)
        {
            loadedCameras.emplace_back(MakeCamera(camera));
        }

        loadedBuffers.reserve(root->buffers.size());
        for (const auto& buffer : root->buffers)
        {
            BufferSource bufferSource;
            bufferSource.root = rootFolder;
            bufferSource.byteLength = buffer.byteLength;
            bufferSource.filename = buffer.uri;
            loadedBuffers.emplace_back(bufferSource);
        }

        loadedSamplers.reserve(root->samplers.size());
        for (const auto& sampler : root->samplers)
        {
            loadedSamplers.emplace_back(MakeSampler(sampler));
        }

        loadedTextures.reserve(root->textures.size());
        for (const auto& texture : root->textures)
        {
            loadedTextures.emplace_back(MakeTexture(texture));
        }


        loadedMaterials.reserve(root->materials.size());
        for (const auto& material : root->materials)
        {
            loadedMaterials.emplace_back(MakePbrMaterial(material));
        }

        loadedMeshes.reserve(root->meshes.size());
        for (const auto& mesh : root->meshes)
        {
            loadedMeshes.emplace_back(MakeMesh(mesh));
        }
    }

    RefPtr<Scene> glTFLoader::MakeScene(const glTF::Scene& scene)
    {
        const String& name = scene.name;

        RefPtrVector<Node> nodes;
        nodes.reserve(scene.nodes.size());

        for (auto nodeIndex : scene.nodes)
        {
            const auto& node = root->nodes[nodeIndex];
            nodes.emplace_back(MakeNode(node));
        }

        return MakeRef<Scene>(move(nodes), name);
    }

    RefPtr<Node> glTFLoader::MakeNode(const glTF::Node& node)
    {
        const String& name = node.name;

        RefPtrVector<Node> children;
        children.reserve(node.children.size());
        for (auto nodeIndex : node.children)
        {
            const auto& node = root->nodes[nodeIndex];
            children.emplace_back(MakeNode(node));
        }

        RefPtr<Mesh> mesh;
        if (node.mesh.has_value())
        {
            mesh = loadedMeshes[node.mesh.value()];
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
                node.rotation.value_or(Quaternion{}),
                node.scale.value_or(Vec3{1.0f, 1.0f, 1.0f}),
                node.translation.value_or(Vec3{0, 0,0 })
            };
        }

        return MakeRef<Node>(move(children), mesh, camera, transform, name);
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
            attribute.semantic = (AttributeSemantic)glTFAttribute.semantic;
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
                    auto hostBuffer = ReadHostBuffer(bufferSource);
                    vb = MakeRef<VertexBuffer>(
                        move(hostBuffer),
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
        auto hostBuffer = ReadHostBuffer(bufferSource);

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
            hostBuffer,
            indexType,
            indices.count,
            bufferView.byteOffset,
            bufferView.byteLength
            );
    }

    RefPtr<PbrMaterial> glTFLoader::MakePbrMaterial(const glTF::Material& glTFMaterial)
    {
        RefPtr<PbrMaterial> material = MakeRef<PbrMaterial>();

        material->name = glTFMaterial.name;

        material->metallicRoughness.baseColorFactor = glTFMaterial.pbrMetallicRoughness.baseColorFactor;
        material->metallicRoughness.metallicFactor = glTFMaterial.pbrMetallicRoughness.metallicFactor;
        material->metallicRoughness.roughnessFactor = glTFMaterial.pbrMetallicRoughness.roughnessFactor;
        
        if (glTFMaterial.pbrMetallicRoughness.baseColorTexture.has_value())
        {
            const auto& textureInfo = glTFMaterial.pbrMetallicRoughness.baseColorTexture.value();
            material->metallicRoughness.baseColorTexture.texCoord = textureInfo.texCoord;
            material->metallicRoughness.baseColorTexture.texture = loadedTextures[textureInfo.index];
        }

        if (glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.has_value())
        {
            const auto& textureInfo = glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.value();
            material->metallicRoughness.metallicRoughnessTexture.texCoord = textureInfo.texCoord;
            material->metallicRoughness.metallicRoughnessTexture.texture = loadedTextures[textureInfo.index];
        }

        if (glTFMaterial.normalTexture.has_value())
        {
            const auto& textureInfo = glTFMaterial.normalTexture.value();
            material->normalTexture.texCoord = textureInfo.texCoord;
            material->normalTexture.texture = loadedTextures[textureInfo.index];
            material->normalTexture.scale = textureInfo.scale;
        }

        if (glTFMaterial.occlusionTexture.has_value())
        {
            const auto& textureInfo = glTFMaterial.occlusionTexture.value();
            material->occlusionTexture.texCoord = textureInfo.texCoord;
            material->occlusionTexture.texture = loadedTextures[textureInfo.index];
            material->occlusionTexture.strength = textureInfo.strength;
        }

        if (glTFMaterial.emissiveTexture.has_value())
        {
            const auto& textureInfo = glTFMaterial.emissiveTexture.value();
            material->emissiveTexture.texCoord = textureInfo.texCoord;
            material->emissiveTexture.texture = loadedTextures[textureInfo.index];
        }

        material->emissiveFactor = glTFMaterial.emissiveFactor;
        material->alphaMode = (AlphaMode)glTFMaterial.alphaMode; // TODO
        material->alphaCutoff = glTFMaterial.alphaCutoff;
        material->doubleSided = glTFMaterial.doubleSided;

        return material;
    }

    RefPtr<Texture> glTFLoader::MakeTexture(const glTF::Texture& texture)
    {
        const auto& name = texture.name;

        RefPtr<Sampler> sampler;
        if (texture.sampler.has_value())
        {
            sampler = loadedSamplers[texture.sampler.value()];
        }

        TextureSource source;
        if (texture.source.has_value())
        {
            const auto& image = root->images[texture.source.value()];
            source.root = rootFolder;
            source.filename = image.uri;
        }

        RefPtr<Image> image = ReadHostImage(source);

        return MakeRef<Texture>(sampler, image, name);
    }

    RefPtr<Sampler> glTFLoader::MakeSampler(const glTF::Sampler& sampler)
    {
        const auto& name = sampler.name;

        vk::SamplerCreateInfo samplerCreateInfo;

        samplerCreateInfo.addressModeU = ToVkSamplerAddresMode(sampler.wrapS);
        samplerCreateInfo.addressModeV = ToVkSamplerAddresMode(sampler.wrapT);

        if (sampler.minFilter.has_value())
        {
            MinFilter minFilter = ToVkMinFilter(sampler.minFilter.value());
            samplerCreateInfo.minFilter = minFilter.minFilter;
            samplerCreateInfo.mipmapMode = minFilter.mipmapMode;
            samplerCreateInfo.minLod = minFilter.minLod;
            samplerCreateInfo.maxLod = minFilter.maxLod;
        }
        
        if (sampler.magFilter.has_value())
        {
            samplerCreateInfo.magFilter = ToVkMagFilter(sampler.magFilter.value());
        }

        return MakeRef<Sampler>(samplerCreateInfo, name);
    }
    Vector<uint8> glTFLoader::ReadHostBuffer(const BufferSource& source)
    {
        UniquePtr<FileStream> stream = MakeUnique<FileStream>(source.root + "/" + source.filename, FileOpenModes::Read);
        Vector<uint8> buffer;
        buffer.resize(source.byteLength);

        stream->Read(buffer.data(), source.byteLength, 1);

        return move(buffer);
    }

    RefPtr<Image> glTFLoader::ReadHostImage(const TextureSource& source)
    {
        return Image::LoadFromFile(source.root + "/" + source.filename);
    }
}
