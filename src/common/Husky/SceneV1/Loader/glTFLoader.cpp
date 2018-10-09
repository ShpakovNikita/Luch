#include <Husky/SceneV1/Loader/glTFLoader.h>
#include <Husky/SceneV1/Loader/LoaderUtils.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/UniquePtr.h>
#include <Husky/FileStream.h>
#include <Husky/SceneV1/Buffer.h>
#include <Husky/SceneV1/Scene.h>
#include <Husky/SceneV1/SceneProperties.h>
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
#include <cstring>
#include <cmath>

namespace Husky::SceneV1::Loader
{
    Map<glTF::ComponentType, ComponentType> ComponentTypes = 
    {
        { glTF::ComponentType::Int8, ComponentType::Int8 },
        { glTF::ComponentType::UInt8, ComponentType::UInt8 },
        { glTF::ComponentType::Int16, ComponentType::Int16 },
        { glTF::ComponentType::UInt16, ComponentType::UInt16 },
        { glTF::ComponentType::UInt, ComponentType::UInt },
        { glTF::ComponentType::Float, ComponentType::Float },
    };

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

            loadedBuffers.emplace_back(ReadHostBuffer(bufferSource));
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

        SceneProperties sceneProperties;

        for (auto nodeIndex : scene.nodes)
        {
            const auto& node = root->nodes[nodeIndex];
            nodes.emplace_back(MakeNode(node, &sceneProperties));
        }

        return MakeRef<Scene>(move(nodes), std::move(sceneProperties), name);
    }

    RefPtr<Node> glTFLoader::MakeNode(const glTF::Node& node, SceneProperties* sceneProperties)
    {
        const String& name = node.name;

        RefPtrVector<Node> children;
        children.reserve(node.children.size());
        for (auto nodeIndex : node.children)
        {
            const auto& node = root->nodes[nodeIndex];
            children.emplace_back(MakeNode(node, sceneProperties));
        }

        RefPtr<Mesh> mesh;
        if (node.mesh.has_value())
        {
            HUSKY_ASSERT_MSG(!node.camera.has_value(), "Node should have either mesh or camera, not both");

            mesh = loadedMeshes[*node.mesh];
            sceneProperties->meshes.insert(mesh);

            for (const auto& primitive : mesh->GetPrimitives())
            {
                sceneProperties->primitives.insert(primitive);

                for(const auto& vertexBuffer : primitive->GetVertexBuffers())
                {
                    sceneProperties->buffers.insert(vertexBuffer.backingBuffer);
                }

                if(primitive->GetIndexBuffer().has_value())
                {
                    sceneProperties->buffers.insert(primitive->GetIndexBuffer()->backingBuffer);
                }

                const auto& material = primitive->GetMaterial();
                if (material != nullptr)
                {
                    sceneProperties->materials.insert(material);

                    if (material->metallicRoughness.baseColorTexture.texture != nullptr)
                    {
                        sceneProperties->textures.insert(material->metallicRoughness.baseColorTexture.texture);
                        sceneProperties->samplers.insert(material->metallicRoughness.baseColorTexture.texture->GetSampler());
                    }

                    if (material->metallicRoughness.metallicRoughnessTexture.texture != nullptr)
                    {
                        sceneProperties->textures.insert(material->metallicRoughness.metallicRoughnessTexture.texture);
                        sceneProperties->samplers.insert(material->metallicRoughness.metallicRoughnessTexture.texture->GetSampler());
                    }

                    if (material->normalTexture.texture != nullptr)
                    {
                        sceneProperties->textures.insert(material->normalTexture.texture);
                        sceneProperties->samplers.insert(material->normalTexture.texture->GetSampler());
                    }

                    if (material->occlusionTexture.texture != nullptr)
                    {
                        sceneProperties->textures.insert(material->occlusionTexture.texture);
                        sceneProperties->samplers.insert(material->occlusionTexture.texture->GetSampler());
                    }

                    if (material->emissiveTexture.texture != nullptr)
                    {
                        sceneProperties->textures.insert(material->emissiveTexture.texture);
                        sceneProperties->samplers.insert(material->emissiveTexture.texture->GetSampler());
                    }

                    sceneProperties->samplers.erase(RefPtr<Sampler>());
                }
            }
        }

        RefPtr<Camera> camera;
        if (node.camera.has_value())
        {
            HUSKY_ASSERT_MSG(!node.mesh.has_value(), "Node should have either mesh or camera, not both");
            auto cameraIndex = *node.camera;
            camera = loadedCameras[cameraIndex];
            sceneProperties->cameras.insert(camera);
        }

        // TODO skin

        Node::TransformType transform;

        if(node.matrix.has_value())
        {
            transform = *node.matrix;
        }
        else
        {
            transform = TransformProperties
            {
                node.rotation.value_or(Quaternion{}),
                node.scale.value_or(Vec3{ 1.0f, 1.0f, 1.0f }),
                node.translation.value_or(Vec3{ 0, 0, 0 })
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
            if(interleave)
            {
                primitives.emplace_back(MakePrimitiveInterleaved(primitive));
            }
            else
            {
                primitives.emplace_back(MakePrimitive(primitive));
            }
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
            VertexBuffer buffer;
            int32 index;
        };

        Map<int32, BufferWithIndex> vertexBuffersMap;

        int32 currentVertexBufferIndex = 0;
        attributes.reserve(primitive.attributes.size());
        for (const auto& glTFAttribute : primitive.attributes)
        {
            const auto& accessor = root->accessors[glTFAttribute.accessor];

            auto& attribute = attributes.emplace_back();
            attribute.semantic = (AttributeSemantic)glTFAttribute.semantic;
            attribute.componentType = ComponentTypes.at(accessor.componentType);
            attribute.attributeType = (AttributeType)accessor.type;
            attribute.offset = accessor.byteOffset;
            attribute.format = AttribuiteToFormat(attribute.attributeType, attribute.componentType);

            const auto& bufferViewIndex = accessor.bufferView;

            VertexBuffer vb;

            if (bufferViewIndex.has_value())
            {
                const auto& bufferView = root->bufferViews[*bufferViewIndex];

                int32 stride;
                if (bufferView.byteStride.has_value())
                {
                    stride = *bufferView.byteStride;
                }
                else
                {
                    stride = CalculateStride(attribute.attributeType, attribute.componentType);
                }

                auto it = vertexBuffersMap.find(*bufferViewIndex);
                if (it != vertexBuffersMap.end())
                {
                    vb = it->second.buffer;
                    attribute.vertexBufferIndex = it->second.index;
                }
                else
                {
                    const auto& buffer = loadedBuffers[bufferView.buffer];

                    vb.backingBuffer = buffer;
                    vb.stride = stride;
                    vb.byteOffset = bufferView.byteOffset;
                    vb.byteLength = bufferView.byteLength;

                    vertexBuffersMap[*bufferViewIndex] = BufferWithIndex{ vb, currentVertexBufferIndex };
                    attribute.vertexBufferIndex = currentVertexBufferIndex;

                    currentVertexBufferIndex++;
                }
            }
        }

        Vector<VertexBuffer> vertexBuffers;
        vertexBuffers.resize(vertexBuffersMap.size());
        for (const auto& kv : vertexBuffersMap)
        {
            vertexBuffers[kv.second.index] = kv.second.buffer;
        }

        Optional<IndexBuffer> indexBuffer;
        if (primitive.indices.has_value())
        {
            indexBuffer = MakeIndexBuffer(root->accessors[*primitive.indices]);
        }

        RefPtr<PbrMaterial> pbrMaterial;
        if (primitive.material.has_value())
        {
            pbrMaterial = loadedMaterials[*primitive.material];
        }

        return MakeRef<Primitive>(
            move(attributes),
            move(vertexBuffers),
            indexBuffer,
            pbrMaterial,
            PrimitiveTopology::TriangleList);
    }

    RefPtr<Primitive> glTFLoader::MakePrimitiveInterleaved(const glTF::Primitive& primitive)
    {
        HUSKY_ASSERT(primitive.mode == glTF::PrimitiveMode::Triangles);

        Vector<PrimitiveAttribute> attributes;
        auto& positionAttribute = attributes.emplace_back();
        positionAttribute.semantic = AttributeSemantic::Position;
        positionAttribute.componentType = ComponentType::Float;
        positionAttribute.attributeType = AttributeType::Vec4;
        positionAttribute.format = Format::R32G32B32A32Sfloat;
        positionAttribute.offset = offsetof(InterleavedVertex, position);
        positionAttribute.vertexBufferIndex = 0;

        auto& normalAttribute = attributes.emplace_back();
        normalAttribute.semantic = AttributeSemantic::Normal;
        normalAttribute.componentType = ComponentType::Float;
        normalAttribute.attributeType = AttributeType::Vec4;
        normalAttribute.format = Format::R32G32B32A32Sfloat;
        normalAttribute.offset = offsetof(InterleavedVertex, normal);
        normalAttribute.vertexBufferIndex = 0;

        auto& tangentAttribute = attributes.emplace_back();
        tangentAttribute.semantic = AttributeSemantic::Tangent;
        tangentAttribute.componentType = ComponentType::Float;
        tangentAttribute.attributeType = AttributeType::Vec4;
        tangentAttribute.format = Format::R32G32B32A32Sfloat;
        tangentAttribute.offset = offsetof(InterleavedVertex, tangent);
        tangentAttribute.vertexBufferIndex = 0;

        auto& texcoordAttribute = attributes.emplace_back();
        texcoordAttribute.semantic = AttributeSemantic::Texcoord_0;
        texcoordAttribute.componentType = ComponentType::Float;
        texcoordAttribute.attributeType = AttributeType::Vec2;
        texcoordAttribute.format = Format::R32G32Sfloat;
        texcoordAttribute.offset = offsetof(InterleavedVertex, texcoord);
        texcoordAttribute.vertexBufferIndex = 0;

        UnorderedMap<AttributeSemantic, glTF::Attribute> glTFAttributes;
        Optional<int32> vertexCount;
        for (const auto& glTFAttribute : primitive.attributes)
        {
            auto semantic = (AttributeSemantic)glTFAttribute.semantic;
            const auto& accessor = root->accessors[glTFAttribute.accessor];
            if(vertexCount.has_value())
            {
                HUSKY_ASSERT(accessor.count == *vertexCount);
            }
            else
            {
                vertexCount = accessor.count;
            }
            glTFAttributes[semantic] = glTFAttribute;
        }

        HUSKY_ASSERT(glTFAttributes.count(AttributeSemantic::Position) == 1);
        HUSKY_ASSERT(glTFAttributes.count(AttributeSemantic::Normal) == 1);
        HUSKY_ASSERT(glTFAttributes.count(AttributeSemantic::Texcoord_0) == 1);

        Vector<Byte> vertexBytes;
        vertexBytes.resize(*vertexCount * sizeof(InterleavedVertex));

        Optional<IndexBuffer> indexBuffer;

        Vector<Vec3> positions;
        positions.resize(*vertexCount);

        Vector<Vec3> normals;
        normals.resize(*vertexCount);

        Vector<Vec4> tangents;
        tangents.resize(*vertexCount);

        Vector<Vec2> texcoords;
        texcoords.resize(*vertexCount);

        Vector<uint32> indices;
        if (primitive.indices.has_value())
        {
            indexBuffer = MakeIndexBuffer(root->accessors[*primitive.indices]);
            indexBuffer->backingBuffer->ReadToHost();

            indices.resize(indexBuffer->count);
            const Byte* indexBytes = indexBuffer->backingBuffer->GetHostBuffer().data() + indexBuffer->byteOffset;
            for(int32 i = 0; i < indexBuffer->count; i++)
            {
                if(indexBuffer->indexType == Graphics::IndexType::UInt16)
                {
                    const uint16* index = reinterpret_cast<const uint16*>(indexBytes) + i;
                    indices.push_back(*index);
                }
                else if(indexBuffer->indexType == Graphics::IndexType::UInt32)
                {
                    const uint32* index = reinterpret_cast<const uint32*>(indexBytes) + i;
                    indices.push_back(*index);
                }
                else
                {
                    HUSKY_ASSERT(false);
                }
            }
        }
        else
        {
            indices.resize(*vertexCount);
            for(int32 i = 0; i < *vertexCount; i++)
            {
                indices[i] = i;
            }
        }

        for(const auto& kv : glTFAttributes)
        {
            AttributeSemantic semantic = kv.first;
            const auto& glTFAttribute = kv.second;

            const auto& accessor = root->accessors[glTFAttribute.accessor];
            HUSKY_ASSERT(accessor.bufferView.has_value());
            const auto& bufferView = root->bufferViews[*accessor.bufferView];
            const auto& buffer = loadedBuffers[bufferView.buffer];

            buffer->ReadToHost();

            const auto& bufferBytes = buffer->GetHostBuffer();

            const Byte* attributeBytesStart = bufferBytes.data() + bufferView.byteOffset;
            for(int32 i = 0; i < *vertexCount; i++)
            {
                auto attributeType = (AttributeType)accessor.type;
                auto componentType = ComponentTypes.at(accessor.componentType);
                int32 stride = CalculateStride(attributeType, componentType);

                const Byte* attributeBytes =
                      attributeBytesStart
                    + accessor.byteOffset
                    + i * bufferView.byteStride.value_or(stride);

                switch(semantic)
                {
                case AttributeSemantic::Position:
                {
                    HUSKY_ASSERT(attributeType == AttributeType::Vec3);
                    HUSKY_ASSERT(componentType == ComponentType::Float);
                    std::memcpy(&positions[i], attributeBytes, sizeof(Vec3));
                    break;
                }
                case AttributeSemantic::Normal:
                {
                    HUSKY_ASSERT(attributeType == AttributeType::Vec3);
                    HUSKY_ASSERT(componentType == ComponentType::Float);
                    std::memcpy(&normals[i], attributeBytes, sizeof(Vec3));
                    break;
                }
                case AttributeSemantic::Tangent:
                {
                    HUSKY_ASSERT(attributeType == AttributeType::Vec4);
                    HUSKY_ASSERT(componentType == ComponentType::Float);
                    std::memcpy(&tangents[i], attributeBytes, sizeof(Vec4));
                    break;
                }
                case AttributeSemantic::Texcoord_0:
                {
                    HUSKY_ASSERT(attributeType == AttributeType::Vec2);

                    if(accessor.componentType == glTF::ComponentType::Float)
                    {
                        std::memcpy(&texcoords[i], attributeBytes, sizeof(Vec2));
                    }
                    else if(accessor.componentType == glTF::ComponentType::UInt8)
                    {
                        Array<uint8, 2> uv;
                        std::memcpy(&uv, attributeBytes, sizeof(uint8) * 2);
                        texcoords[i].x = (float32)uv[0] / Limits<uint8>::max();
                        texcoords[i].y = (float32)uv[1] / Limits<uint8>::max();
                    }
                    else if(accessor.componentType == glTF::ComponentType::UInt16)
                    {
                        Array<uint16, 2> uv;
                        std::memcpy(&uv, attributeBytes, sizeof(uint16) * 2);
                        texcoords[i].x = (float32)uv[0] / Limits<uint16>::max();
                        texcoords[i].y = (float32)uv[1] / Limits<uint16>::max();
                    }
                    else
                    {
                        HUSKY_ASSERT_MSG(false, "Invalid texcoord component type");
                    }
                    break;
                }
                default:
                    HUSKY_ASSERT_MSG(false, "Unknown semantic for interleaved");
                }
            }
        }

        bool hasTangent = glTFAttributes.count(AttributeSemantic::Tangent) == 1;
        if(!hasTangent)
        {
            GenerateTangents(
                primitive.mode,
                indices.size(),
                indices.data(),
                positions.data(),
                normals.data(),
                texcoords.data(),
                tangents.data());
        }

        for(int32 i = 0; i < *vertexCount; i++)
        {
            InterleavedVertex vertex;
            vertex.position = positions[i];
            vertex.normal = normals[i];
            vertex.tangent = tangents[i];
            vertex.texcoord = texcoords[i];
            std::memcpy(vertexBytes.data() + i * sizeof(InterleavedVertex), &vertex, sizeof(InterleavedVertex));
        }

        RefPtr<Buffer> buffer = MakeRef<Buffer>();
        buffer->SetHostBuffer(std::move(vertexBytes));

        Vector<VertexBuffer> vertexBuffers;
        auto& vertexBuffer = vertexBuffers.emplace_back();
        vertexBuffer.backingBuffer = buffer;
        vertexBuffer.stride = sizeof(InterleavedVertex);
        vertexBuffer.byteOffset = 0;
        vertexBuffer.byteLength = vertexBytes.size();

        RefPtr<PbrMaterial> pbrMaterial;
        if (primitive.material.has_value())
        {
            pbrMaterial = loadedMaterials[*primitive.material];
        }

        return MakeRef<Primitive>(
            move(attributes),
            move(vertexBuffers),
            indexBuffer,
            pbrMaterial,
            PrimitiveTopology::TriangleList);
    }

    Optional<IndexBuffer> glTFLoader::MakeIndexBuffer(const glTF::Accessor& indices)
    {
        if (!indices.bufferView.has_value())
        {
            return {};
        }

        const auto& bufferView = root->bufferViews[*indices.bufferView];
        HUSKY_ASSERT_MSG(!bufferView.byteStride.has_value(), "index accessor buffer views should not have stride");

        auto buffer = loadedBuffers[bufferView.buffer];

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
            HUSKY_ASSERT_MSG(false, "Unsupported index type");
        }

        IndexBuffer ib;
        ib.backingBuffer = buffer;
        ib.indexType = indexType;
        ib.count = indices.count;
        ib.byteOffset = bufferView.byteOffset;
        ib.byteLength = bufferView.byteLength;

        return ib;
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
            const auto& textureInfo = *glTFMaterial.pbrMetallicRoughness.baseColorTexture;
            material->metallicRoughness.baseColorTexture.texCoord = textureInfo.texCoord;
            material->metallicRoughness.baseColorTexture.texture = loadedTextures[textureInfo.index];
        }

        if (glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture;
            material->metallicRoughness.metallicRoughnessTexture.texCoord = textureInfo.texCoord;
            material->metallicRoughness.metallicRoughnessTexture.texture = loadedTextures[textureInfo.index];
        }

        if (glTFMaterial.normalTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.normalTexture;
            material->normalTexture.texCoord = textureInfo.texCoord;
            material->normalTexture.texture = loadedTextures[textureInfo.index];
            material->normalTexture.scale = textureInfo.scale;
        }

        if (glTFMaterial.occlusionTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.occlusionTexture;
            material->occlusionTexture.texCoord = textureInfo.texCoord;
            material->occlusionTexture.texture = loadedTextures[textureInfo.index];
            material->occlusionTexture.strength = textureInfo.strength;
        }

        if (glTFMaterial.emissiveTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.emissiveTexture;
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
            sampler = loadedSamplers[*texture.sampler];
        }

        TextureSource source;
        if (texture.source.has_value())
        {
            const auto& image = root->images[*texture.source];
            source.root = rootFolder;
            source.filename = image.uri;
        }

        RefPtr<Image> image = ReadHostImage(source);

        return MakeRef<Texture>(sampler, image, name);
    }

    RefPtr<Sampler> glTFLoader::MakeSampler(const glTF::Sampler& sampler)
    {
        const auto& name = sampler.name;

        Graphics::SamplerCreateInfo samplerCreateInfo;

        samplerCreateInfo.uAddressMode = ToSamplerAddresMode(sampler.wrapS);
        samplerCreateInfo.vAddressMode = ToSamplerAddresMode(sampler.wrapT);

        if (sampler.minFilter.has_value())
        {
            MinFilter minFilter = ToMinFilter(*sampler.minFilter);
            samplerCreateInfo.minFilter = minFilter.minFilter;
            samplerCreateInfo.mipFilter = minFilter.mipFilter;
            samplerCreateInfo.minLod = minFilter.minLod;
            samplerCreateInfo.maxLod = minFilter.maxLod;
        }
        
        if (sampler.magFilter.has_value())
        {
            samplerCreateInfo.magFilter = ToMagFilter(*sampler.magFilter);
        }

        return MakeRef<Sampler>(samplerCreateInfo, name);
    }

    RefPtr<Buffer> glTFLoader::ReadHostBuffer(const BufferSource& source)
    {
        auto buffer = MakeRef<Buffer>(source);
        buffer->ReadToHost();
        return buffer;
    }

    RefPtr<Image> glTFLoader::ReadHostImage(const TextureSource& source)
    {
        return Image::LoadFromFile(source.root + "/" + source.filename);
    }
}
