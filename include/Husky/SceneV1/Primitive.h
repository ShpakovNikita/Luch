#pragma once

#include <Husky/PrimitiveTopology.h>
#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/SceneV1/AttributeSemantic.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    class IndexBuffer;
    class PbrMaterial;

    enum class ComponentType
    {
        Int8,
        UInt8,
        Int16,
        UInt16,
        UInt,
        Float,
    };

    enum class AttributeType
    {
        Scalar,
        Vec2,
        Vec3,
        Vec4,
        Mat2x2,
        Mat3x3,
        Mat4x4,
    };

    struct PrimitiveAttribute
    {
        AttributeSemantic attributeSemantic;
        ComponentType componentType;
        AttributeType attributeType;
        int32 offset = 0;
        int32 vertexBufferIndex;
    };

    class Primitive
    {
    public:
        Primitive(
            Vector<PrimitiveAttribute>&& attributes,
            RefPtrVector<VertexBuffer>&& vertexBuffers,
            const RefPtr<IndexBuffer>& indexBuffer,
            const RefPtr<PbrMaterial>& material,
            PrimitiveTopology topology
        );

        ~Primitive();

        inline PrimitiveTopology GetTopology() const { return topology; }
    private:
        Vector<PrimitiveAttribute> attributes;
        RefPtrVector<VertexBuffer> vertexBuffers;
        RefPtr<IndexBuffer> indexBuffer;
        RefPtr<PbrMaterial> material;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    };
}
