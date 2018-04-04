#pragma once

#include <Husky/PrimitiveTopology.h>
#include <Husky/Types.h>
#include <Husky/Format.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
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
        AttributeSemantic semantic;
        ComponentType componentType;
        AttributeType attributeType;
        Format format;
        int32 offset = 0;
        int32 vertexBufferIndex;
    };

    class Primitive : public BaseObject
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

        const Vector<PrimitiveAttribute>& GetAttributes() const { return attributes; }
        const RefPtrVector<VertexBuffer>& GetVertexBuffers() const { return vertexBuffers; }
        const RefPtr<IndexBuffer>& GetIndexBuffer() const { return indexBuffer; }
        const RefPtr<PbrMaterial>& GetMaterial() const { return material; }
        inline PrimitiveTopology GetTopology() const { return topology; }
        inline const RefPtr<Vulkan::Pipeline>& GetPipeline() const { return pipeline; }
        inline void SetPipeline(const RefPtr<Vulkan::Pipeline>& aPipeline) { pipeline = aPipeline; }
    private:
        Vector<PrimitiveAttribute> attributes;
        RefPtrVector<VertexBuffer> vertexBuffers;
        RefPtr<IndexBuffer> indexBuffer;
        RefPtr<PbrMaterial> material;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        RefPtr<Vulkan::Pipeline> pipeline;
    };
}