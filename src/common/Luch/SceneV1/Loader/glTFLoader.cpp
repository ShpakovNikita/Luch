#include <Luch/SceneV1/Loader/glTFLoader.h>
#include <Luch/SceneV1/Loader/LoaderUtils.h>
#include <Luch/glTF2/glTF.h>
#include <Luch/UniquePtr.h>
#include <Luch/FileStream.h>
#include <Luch/SceneV1/Buffer.h>
#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/SceneProperties.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Image.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/BufferSource.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/VertexBuffer.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/Render/Common.h>
#include <cstring>
#include <cmath>

namespace Luch::SceneV1::Loader
{
    UnorderedMap<glTF::ComponentType, ComponentType> ComponentTypes =
    {
        { glTF::ComponentType::Int8, ComponentType::Int8 },
        { glTF::ComponentType::UInt8, ComponentType::UInt8 },
        { glTF::ComponentType::Int16, ComponentType::Int16 },
        { glTF::ComponentType::UInt16, ComponentType::UInt16 },
        { glTF::ComponentType::UInt, ComponentType::UInt },
        { glTF::ComponentType::Float, ComponentType::Float },
    };

    UnorderedMap<glTF::LightType, LightType> LightTypes =
    {
        { glTF::LightType::Spot, LightType::Spot },
        { glTF::LightType::Directional, LightType::Directional },
        { glTF::LightType::Point, LightType::Point },
    };

    glTFLoader::glTFLoader(const String& aRootFolder, SharedPtr<glTF::glTFRoot> glTFRoot)
        : rootFolder(aRootFolder)
        , root(move(glTFRoot))
    {
    }

    int32 glTFLoader::GetSceneCount() const
    {
        return root->scenes.size();
    }

    RefPtr<Scene> glTFLoader::LoadScene(int32 index)
    {
        auto context = LoadProperties();
        auto scene = MakeScene(root->scenes[index], context);
        return scene;
    }

    SceneLoadContext glTFLoader::LoadProperties()
    {
        SceneLoadContext context;

        context.loadedCameras.reserve(root->cameras.size());
        for (const auto& camera : root->cameras)
        {
            context.loadedCameras.emplace_back(MakeCamera(camera, context));
        }

        context.loadedBuffers.reserve(root->buffers.size());
        for (const auto& buffer : root->buffers)
        {
            BufferSource bufferSource;
            bufferSource.root = rootFolder;
            bufferSource.byteLength = buffer.byteLength;
            bufferSource.filename = buffer.uri;

            context.loadedBuffers.emplace_back(ReadHostBuffer(bufferSource));
        }

        context.loadedSamplers.reserve(root->samplers.size());
        for (const auto& sampler : root->samplers)
        {
            context.loadedSamplers.emplace_back(MakeSampler(sampler, context));
        }

        context.loadedTextures.reserve(root->textures.size());
        for (const auto& texture : root->textures)
        {
            context.loadedTextures.emplace_back(MakeTexture(texture, context));
        }

        context.loadedMaterials.reserve(root->materials.size());
        for (const auto& material : root->materials)
        {
            context.loadedMaterials.emplace_back(MakePbrMaterial(material, context));
        }

        context.loadedMeshes.reserve(root->meshes.size());
        for (const auto& mesh : root->meshes)
        {
            context.loadedMeshes.emplace_back(MakeMesh(mesh, context));
        }

        if(root->extensions.has_value())
        {
            if(root->extensions->lights.has_value())
            {
                context.loadedLights.reserve(root->extensions->lights->lights.size());
                for(const auto& light : root->extensions->lights->lights)
                {
                    context.loadedLights.emplace_back(MakeLight(light, context));
                }
            }
        }

        return context;
    }

    RefPtr<Scene> glTFLoader::MakeScene(const glTF::Scene& glTFScene, const SceneLoadContext& context)
    {
        auto scene = MakeRef<Scene>();
        scene->SetName(glTFScene.name);

        for (auto nodeIndex : glTFScene.nodes)
        {
            const auto& glTFNode = root->nodes[nodeIndex];
            auto node = MakeNode(glTFNode, context);
            scene->AddNode(node);
        }

        return scene;
    }

    RefPtr<Node> glTFLoader::MakeNode(const glTF::Node& glTFNode, const SceneLoadContext& context)
    {
        auto node = MakeRef<Node>();
        node->SetName(glTFNode.name);

        RefPtrVector<Node> children;
        children.reserve(glTFNode.children.size());
        for (auto nodeIndex : glTFNode.children)
        {
            const auto& glTFChildNode = root->nodes[nodeIndex];
            auto childNode = MakeNode(glTFChildNode, context);
            node->AddChild(childNode);
            children.emplace_back(childNode);
        }

        if (glTFNode.mesh.has_value())
        {
            const auto& mesh = context.loadedMeshes[*glTFNode.mesh];
            node->SetMesh(mesh);
        }

        if (glTFNode.camera.has_value())
        {
            auto cameraIndex = *glTFNode.camera;
            const auto& camera = context.loadedCameras[cameraIndex];
            node->SetCamera(camera);
        }

        RefPtr<Light> light;
        if(glTFNode.extensions.has_value() && glTFNode.extensions->lights.has_value())
        {
            auto lightIndex = glTFNode.extensions->lights->light;
            light = context.loadedLights[lightIndex];
            node->SetLight(light);
        }

        // TODO skin

        Node::TransformType transform;

        if(glTFNode.matrix.has_value())
        {
            transform = *glTFNode.matrix;
        }
        else
        {
            transform = TransformProperties
            {
                glTFNode.rotation.value_or(Quaternion{}),
                glTFNode.scale.value_or(Vec3{ 1.0f, 1.0f, 1.0f }),
                glTFNode.translation.value_or(Vec3{ 0, 0, 0 })
            };
        }

        node->SetLocalTransform(transform);

        return node;
    }

    RefPtr<Mesh> glTFLoader::MakeMesh(const glTF::Mesh& mesh, const SceneLoadContext& context)
    {
        const auto& name = mesh.name;

        RefPtrVector<Primitive> primitives;

        for (const auto& primitive : mesh.primitives)
        {
            if(interleave)
            {
                primitives.emplace_back(MakePrimitiveInterleaved(primitive, context));
            }
            else
            {
                primitives.emplace_back(MakePrimitive(primitive, context));
            }
        }

        return MakeRef<Mesh>(move(primitives), name);
    }

    RefPtr<Camera> glTFLoader::MakeCamera(const glTF::Camera& camera, const SceneLoadContext& context)
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
        auto result = MakeRef<PerspectiveCamera>(camera.yfov, camera.znear, camera.zfar, camera.aspectRatio);
        result->SetName(name);
        return result;
    }

    RefPtr<OrthographicCamera> glTFLoader::MakeOrthographicCamera(const String& name, const glTF::Orthographic& camera)
    {
        auto result = MakeRef<OrthographicCamera>(camera.xmag, camera.ymag, camera.zfar, camera.znear);
        result->SetName(name);
        return result;
    }

    RefPtr<Primitive> glTFLoader::MakePrimitive(const glTF::Primitive& glTFPrimitive, const SceneLoadContext& context)
    {
        auto primitive = MakeRef<Primitive>();

        Vector<PrimitiveAttribute> attributes;

        struct BufferWithIndex
        {
            VertexBuffer buffer;
            int32 index;
        };

        Map<int32, BufferWithIndex> vertexBuffersMap;

        int32 currentVertexBufferIndex = 0;
        attributes.reserve(glTFPrimitive.attributes.size());
        for (const auto& glTFAttribute : glTFPrimitive.attributes)
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
                    const auto& buffer = context.loadedBuffers[bufferView.buffer];

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

        primitive->SetAttributes(std::move(attributes));

        Vector<VertexBuffer> vertexBuffers;
        vertexBuffers.resize(vertexBuffersMap.size());
        for (const auto& kv : vertexBuffersMap)
        {
            vertexBuffers[kv.second.index] = kv.second.buffer;
        }

        primitive->SetVertexBuffers(std::move(vertexBuffers));

        if (glTFPrimitive.indices.has_value())
        {
            auto indexBuffer = MakeIndexBuffer(root->accessors[*glTFPrimitive.indices], context);
            primitive->SetIndexBuffer(indexBuffer);
        }

        if (glTFPrimitive.material.has_value())
        {
            auto pbrMaterial = context.loadedMaterials[*glTFPrimitive.material];
            primitive->SetMaterial(pbrMaterial);
        }

        LUCH_ASSERT(glTFPrimitive.mode == glTF::PrimitiveMode::Triangles);
        primitive->SetTopology(PrimitiveTopology::TriangleList);

        return primitive;
    }

    RefPtr<Primitive> glTFLoader::MakePrimitiveInterleaved(const glTF::Primitive& glTFPrimitive, const SceneLoadContext& context)
    {
        auto primitive = MakeRef<Primitive>();

        LUCH_ASSERT(glTFPrimitive.mode == glTF::PrimitiveMode::Triangles);

        Vector<PrimitiveAttribute> attributes;
        auto& positionAttribute = attributes.emplace_back();
        positionAttribute.semantic = AttributeSemantic::Position;
        positionAttribute.componentType = ComponentType::Float;
        positionAttribute.attributeType = AttributeType::Vec4;
        positionAttribute.format = Format::R32G32B32A32Sfloat;
        positionAttribute.offset = offsetof(Render::Vertex, position);
        positionAttribute.vertexBufferIndex = 0;

        auto& normalAttribute = attributes.emplace_back();
        normalAttribute.semantic = AttributeSemantic::Normal;
        normalAttribute.componentType = ComponentType::Float;
        normalAttribute.attributeType = AttributeType::Vec4;
        normalAttribute.format = Format::R32G32B32A32Sfloat;
        normalAttribute.offset = offsetof(Render::Vertex, normal);
        normalAttribute.vertexBufferIndex = 0;

        auto& tangentAttribute = attributes.emplace_back();
        tangentAttribute.semantic = AttributeSemantic::Tangent;
        tangentAttribute.componentType = ComponentType::Float;
        tangentAttribute.attributeType = AttributeType::Vec4;
        tangentAttribute.format = Format::R32G32B32A32Sfloat;
        tangentAttribute.offset = offsetof(Render::Vertex, tangent);
        tangentAttribute.vertexBufferIndex = 0;

        auto& texcoordAttribute = attributes.emplace_back();
        texcoordAttribute.semantic = AttributeSemantic::Texcoord_0;
        texcoordAttribute.componentType = ComponentType::Float;
        texcoordAttribute.attributeType = AttributeType::Vec2;
        texcoordAttribute.format = Format::R32G32Sfloat;
        texcoordAttribute.offset = offsetof(Render::Vertex, texcoord);
        texcoordAttribute.vertexBufferIndex = 0;

        primitive->SetAttributes(std::move(attributes));

        UnorderedMap<AttributeSemantic, glTF::Attribute> glTFAttributes;
        Optional<int32> vertexCount;
        for (const auto& glTFAttribute : glTFPrimitive.attributes)
        {
            auto semantic = (AttributeSemantic)glTFAttribute.semantic;
            const auto& accessor = root->accessors[glTFAttribute.accessor];
            if(vertexCount.has_value())
            {
                LUCH_ASSERT(accessor.count == *vertexCount);
            }
            else
            {
                vertexCount = accessor.count;
            }
            glTFAttributes[semantic] = glTFAttribute;
        }

        LUCH_ASSERT(glTFAttributes.count(AttributeSemantic::Position) == 1);
        LUCH_ASSERT(glTFAttributes.count(AttributeSemantic::Normal) == 1);
        LUCH_ASSERT(glTFAttributes.count(AttributeSemantic::Texcoord_0) == 1);

        Vector<Byte> vertexBytes;
        vertexBytes.resize(*vertexCount * sizeof(Render::Vertex));

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
        if (glTFPrimitive.indices.has_value())
        {
            indexBuffer = MakeIndexBuffer(root->accessors[*glTFPrimitive.indices], context);
            indexBuffer->backingBuffer->ReadToHost();
            primitive->SetIndexBuffer(indexBuffer);

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
                    LUCH_ASSERT(false);
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
            LUCH_ASSERT(accessor.bufferView.has_value());
            const auto& bufferView = root->bufferViews[*accessor.bufferView];
            const auto& buffer = context.loadedBuffers[bufferView.buffer];

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
                    LUCH_ASSERT(attributeType == AttributeType::Vec3);
                    LUCH_ASSERT(componentType == ComponentType::Float);
                    std::memcpy(&positions[i], attributeBytes, sizeof(Vec3));
                    break;
                }
                case AttributeSemantic::Normal:
                {
                    LUCH_ASSERT(attributeType == AttributeType::Vec3);
                    LUCH_ASSERT(componentType == ComponentType::Float);
                    std::memcpy(&normals[i], attributeBytes, sizeof(Vec3));
                    break;
                }
                case AttributeSemantic::Tangent:
                {
                    LUCH_ASSERT(attributeType == AttributeType::Vec4);
                    LUCH_ASSERT(componentType == ComponentType::Float);
                    std::memcpy(&tangents[i], attributeBytes, sizeof(Vec4));
                    break;
                }
                case AttributeSemantic::Texcoord_0:
                {
                    LUCH_ASSERT(attributeType == AttributeType::Vec2);

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
                        LUCH_ASSERT_MSG(false, "Invalid texcoord component type");
                    }
                    break;
                }
                default:
                    LUCH_ASSERT_MSG(false, "Unknown semantic for interleaved");
                }
            }
        }

        bool hasTangent = glTFAttributes.count(AttributeSemantic::Tangent) == 1;
        if(!hasTangent)
        {
            GenerateTangents(
                glTFPrimitive.mode,
                indices.size(),
                indices.data(),
                positions.data(),
                normals.data(),
                texcoords.data(),
                tangents.data());
        }

        for(int32 i = 0; i < *vertexCount; i++)
        {
            Render::Vertex vertex;
            vertex.position = positions[i];
            vertex.normal = normals[i];
            vertex.tangent = tangents[i];
            vertex.texcoord = texcoords[i];
            std::memcpy(vertexBytes.data() + i * sizeof(Render::Vertex), &vertex, sizeof(Render::Vertex));
        }

        RefPtr<Buffer> buffer = MakeRef<Buffer>();
        buffer->SetHostBuffer(std::move(vertexBytes));

        Vector<VertexBuffer> vertexBuffers;
        auto& vertexBuffer = vertexBuffers.emplace_back();
        vertexBuffer.backingBuffer = buffer;
        vertexBuffer.stride = sizeof(Render::Vertex);
        vertexBuffer.byteOffset = 0;
        vertexBuffer.byteLength = vertexBytes.size();

        primitive->SetVertexBuffers(vertexBuffers);

        if (glTFPrimitive.material.has_value())
        {
            const auto& pbrMaterial = context.loadedMaterials[*glTFPrimitive.material];
            primitive->SetMaterial(pbrMaterial);
        }

        primitive->SetTopology(PrimitiveTopology::TriangleList);

        return primitive;
    }

    Optional<IndexBuffer> glTFLoader::MakeIndexBuffer(const glTF::Accessor& indices, const SceneLoadContext& context)
    {
        if (!indices.bufferView.has_value())
        {
            return {};
        }

        const auto& bufferView = root->bufferViews[*indices.bufferView];
        LUCH_ASSERT_MSG(!bufferView.byteStride.has_value(), "index accessor buffer views should not have stride");

        auto buffer = context.loadedBuffers[bufferView.buffer];

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
            LUCH_ASSERT_MSG(false, "Unsupported index type");
        }

        IndexBuffer ib;
        ib.backingBuffer = buffer;
        ib.indexType = indexType;
        ib.count = indices.count;
        ib.byteOffset = bufferView.byteOffset;
        ib.byteLength = bufferView.byteLength;

        return ib;
    }

    RefPtr<PbrMaterial> glTFLoader::MakePbrMaterial(const glTF::Material& glTFMaterial, const SceneLoadContext& context)
    {
        RefPtr<PbrMaterial> material = MakeRef<PbrMaterial>();

        material->SetName(glTFMaterial.name);

        PbrMaterialProperties properties;

        properties.metallicRoughness.baseColorFactor = glTFMaterial.pbrMetallicRoughness.baseColorFactor;
        properties.metallicRoughness.metallicFactor = glTFMaterial.pbrMetallicRoughness.metallicFactor;
        properties.metallicRoughness.roughnessFactor = glTFMaterial.pbrMetallicRoughness.roughnessFactor;
        
        if (glTFMaterial.pbrMetallicRoughness.baseColorTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.pbrMetallicRoughness.baseColorTexture;
            properties.metallicRoughness.baseColorTextureInfo.texCoord = textureInfo.texCoord;

            const auto& texture = context.loadedTextures[textureInfo.index];
            texture->SetSRGB(true);
            material->SetBaseColorTexture(texture);
        }

        if (glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture;
            properties.metallicRoughness.metallicRoughnessTextureInfo.texCoord = textureInfo.texCoord;

            const auto& texture = context.loadedTextures[textureInfo.index];
            material->SetMetallicRoughnessTexture(texture);
        }

        if (glTFMaterial.normalTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.normalTexture;
            properties.normalTextureInfo.texCoord = textureInfo.texCoord;
            properties.normalTextureInfo.scale = textureInfo.scale;

            const auto& texture = context.loadedTextures[textureInfo.index];
            material->SetNormalTexture(texture);
        }

        if (glTFMaterial.occlusionTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.occlusionTexture;
            properties.occlusionTextureInfo.texCoord = textureInfo.texCoord;
            properties.occlusionTextureInfo.strength = textureInfo.strength;

            const auto& texture = context.loadedTextures[textureInfo.index];
            material->SetOcclusionTexture(texture);
        }

        if (glTFMaterial.emissiveTexture.has_value())
        {
            const auto& textureInfo = *glTFMaterial.emissiveTexture;
            properties.emissiveTextureInfo.texCoord = textureInfo.texCoord;

            const auto& texture = context.loadedTextures[textureInfo.index];
            texture->SetSRGB(true);
            material->SetEmissiveTexture(texture);
        }

        properties.emissiveFactor = glTFMaterial.emissiveFactor;
        properties.alphaMode = (AlphaMode)glTFMaterial.alphaMode; // TODO
        properties.alphaCutoff = glTFMaterial.alphaCutoff;
        properties.doubleSided = glTFMaterial.doubleSided;

        material->SetProperties(properties);

        return material;
    }

    RefPtr<Texture> glTFLoader::MakeTexture(const glTF::Texture& texture, const SceneLoadContext& context)
    {
        const auto& name = texture.name;

        RefPtr<Sampler> sampler;
        if (texture.sampler.has_value())
        {
            sampler = context.loadedSamplers[*texture.sampler];
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

    RefPtr<Sampler> glTFLoader::MakeSampler(const glTF::Sampler& sampler, const SceneLoadContext& context)
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

    RefPtr<Light> glTFLoader::MakeLight(const glTF::LightPunctual& glTFLight, const SceneLoadContext& context)
    {
        auto lightType = LightTypes.at(glTFLight.type);
        RefPtr<Light> light = MakeRef<Light>();
        light->SetName(glTFLight.name);
        light->SetColor(glTFLight.color);
        light->SetIntensity(glTFLight.intensity);
        light->SetType(lightType);
        light->SetRange(glTFLight.range);
        if(lightType == LightType::Spot)
        {
            LUCH_ASSERT(glTFLight.spot.has_value());
            light->SetInnerConeAngle(glTFLight.spot->innerConeAngle);
            light->SetOuterConeAngle(glTFLight.spot->outerConeAngle);
        }
        return light;
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
