#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/PbrMaterial.h>

namespace Husky::SceneV1
{
    Map<Tuple<AttributeType, ComponentType>, Format> AttributeFormats = 
    {
        { { AttributeType::Scalar, ComponentType::Int8}, Format::R8Sint },
        { { AttributeType::Scalar, ComponentType::UInt8}, Format::R8Uint },
        { { AttributeType::Scalar, ComponentType::Int16}, Format::R16Sint },
        { { AttributeType::Scalar, ComponentType::UInt16}, Format::R16Uint },
        { { AttributeType::Scalar, ComponentType::UInt},  Format::R32Uint },
        { { AttributeType::Scalar, ComponentType::Float}, Format::R32Sfloat },

        { { AttributeType::Vec2, ComponentType::Int8}, Format::R8G8Sint },
        { { AttributeType::Vec2, ComponentType::UInt8}, Format::R8G8Uint },
        { { AttributeType::Vec2, ComponentType::Int16}, Format::R16G16Sint },
        { { AttributeType::Vec2, ComponentType::UInt16}, Format::R16G16Uint },
        { { AttributeType::Vec2, ComponentType::UInt},  Format::R32G32Uint },
        { { AttributeType::Vec2, ComponentType::Float}, Format::R32G32Sfloat },

        { { AttributeType::Vec3, ComponentType::Int8}, Format::R8G8B8Sint },
        { { AttributeType::Vec3, ComponentType::UInt8}, Format::R8G8B8Uint },
        { { AttributeType::Vec3, ComponentType::Int16}, Format::R16G16B16Sint },
        { { AttributeType::Vec3, ComponentType::UInt16}, Format::R16G16B16Uint },
        { { AttributeType::Vec3, ComponentType::UInt},  Format::R32G32B32Uint },
        { { AttributeType::Vec3, ComponentType::Float}, Format::R32G32B32Sfloat },

        { { AttributeType::Vec4, ComponentType::Int8},  Format::R8G8B8A8Sint },
        { { AttributeType::Vec4, ComponentType::UInt8}, Format::R8G8B8A8Uint },
        { { AttributeType::Vec4, ComponentType::Int16}, Format::R16G16B16A16Sint },
        { { AttributeType::Vec4, ComponentType::UInt16},Format::R16G16B16A16Uint },
        { { AttributeType::Vec4, ComponentType::UInt},  Format::R32G32B32A32Uint },
        { { AttributeType::Vec4, ComponentType::Float}, Format::R32G32B32A32Sfloat },
        
        // TODO
    };

    Primitive::Primitive(
        Vector<PrimitiveAttribute>&& aAttributes,
        Vector<VertexBuffer>&& aVertexBuffers,
        const Optional<IndexBuffer>& aIndexBuffer,
        const RefPtr<PbrMaterial>& aMaterial,
        PrimitiveTopology aTopology)
        : attributes(std::move(aAttributes))
        , vertexBuffers(std::move(aVertexBuffers))
        , indexBuffer(aIndexBuffer)
        , material(aMaterial)
        , topology(aTopology)
    {
    }

    Primitive::~Primitive() = default;

    Format AttribuiteToFormat(AttributeType attributeType, ComponentType componentType)
    {
        return AttributeFormats.at({attributeType, componentType});
    }
}
