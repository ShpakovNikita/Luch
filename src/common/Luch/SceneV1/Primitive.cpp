#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Scene.h>

namespace Luch::SceneV1
{
    Map<Tuple<AttributeType, ComponentType>, Format> AttributeFormats = 
    {
        { { AttributeType::Scalar, ComponentType::Int8}, Format::R8Sint },
        { { AttributeType::Scalar, ComponentType::UInt8}, Format::R8Uint },
        { { AttributeType::Scalar, ComponentType::Int16}, Format::R16Sint },
        { { AttributeType::Scalar, ComponentType::UInt16}, Format::R16Uint },
        { { AttributeType::Scalar, ComponentType::UInt},  Format::R32Uint },
        { { AttributeType::Scalar, ComponentType::Float}, Format::R32Sfloat },

        { { AttributeType::Vec2, ComponentType::Int8}, Format::RG8Sint },
        { { AttributeType::Vec2, ComponentType::UInt8}, Format::RG8Uint },
        { { AttributeType::Vec2, ComponentType::Int16}, Format::RG16Sint },
        { { AttributeType::Vec2, ComponentType::UInt16}, Format::RG16Uint },
        { { AttributeType::Vec2, ComponentType::UInt},  Format::RG32Uint },
        { { AttributeType::Vec2, ComponentType::Float}, Format::RG32Sfloat },

        { { AttributeType::Vec3, ComponentType::Int8}, Format::RGB8Sint },
        { { AttributeType::Vec3, ComponentType::UInt8}, Format::RGB8Uint },
        { { AttributeType::Vec3, ComponentType::Int16}, Format::RGB16Sint },
        { { AttributeType::Vec3, ComponentType::UInt16}, Format::RGB16Uint },
        { { AttributeType::Vec3, ComponentType::UInt},  Format::RGB32Uint },
        { { AttributeType::Vec3, ComponentType::Float}, Format::RGB32Sfloat },

        { { AttributeType::Vec4, ComponentType::Int8},  Format::RGBA8Sint },
        { { AttributeType::Vec4, ComponentType::UInt8}, Format::RGBA8Uint },
        { { AttributeType::Vec4, ComponentType::Int16}, Format::RGBA16Sint },
        { { AttributeType::Vec4, ComponentType::UInt16},Format::RGBA16Uint },
        { { AttributeType::Vec4, ComponentType::UInt},  Format::RGBA32Uint },
        { { AttributeType::Vec4, ComponentType::Float}, Format::RGBA32Sfloat },
    };

    Primitive::Primitive() = default;
    Primitive::~Primitive() = default;

    Format AttribuiteToFormat(AttributeType attributeType, ComponentType componentType)
    {
        return AttributeFormats.at({attributeType, componentType});
    }
}
