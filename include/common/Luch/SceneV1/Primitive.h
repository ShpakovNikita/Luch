#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/Rect3.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/SceneV1/AttributeSemantic.h>
#include <Luch/SceneV1/IndexBuffer.h>
#include <Luch/SceneV1/VertexBuffer.h>

namespace Luch::SceneV1
{
    using namespace Graphics;

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

    Format AttribuiteToFormat(AttributeType attributeType, ComponentType componentType);

    struct PrimitiveAttribute
    {
        AttributeSemantic semantic;
        ComponentType componentType;
        AttributeType attributeType;
        Format format = Format::Undefined;
        int32 offset = 0;
        int32 vertexBufferIndex = 0;
    };

    class Primitive : public BaseObject
    {
        friend class Scene;
    public:
        Primitive();
        ~Primitive();

        inline const Vector<PrimitiveAttribute>& GetAttributes() const { return attributes; }
        inline void SetAttributes(Vector<PrimitiveAttribute> aAttributes) { attributes = std::move(aAttributes); }

        inline const Vector<VertexBuffer>& GetVertexBuffers() const { return vertexBuffers; }
        inline void SetVertexBuffers(Vector<VertexBuffer> aVertexBuffers) { vertexBuffers = std::move(aVertexBuffers); }

        inline const Optional<IndexBuffer>& GetIndexBuffer() const { return indexBuffer; }
        inline void SetIndexBuffer(const Optional<IndexBuffer>& aIndexBuffer) { indexBuffer = aIndexBuffer; }

        inline const RefPtr<PbrMaterial>& GetMaterial() const { return material; }
        inline void SetMaterial(const RefPtr<PbrMaterial>& aMaterial) { material = aMaterial; }

        inline PrimitiveTopology GetTopology() const { return topology; }
        inline void SetTopology(PrimitiveTopology aTopology) { topology = aTopology; }

        inline bool HasAABB() const { return aabb.has_value(); }
        inline const Rect3f& GetAABB() const { return *aabb; }
        inline void SetAABB(const Optional<Rect3f>& aAABB) { aabb = aAABB; }
        inline void ResetAABB() { aabb.reset(); }

        inline const RefPtr<Graphics::GraphicsPipelineState>& GetGraphicsPipelineState(const String& key) const
        {
            return graphicsPipelineStates[key];
        }

        inline void SetGraphicsPipelineState(const String& key, const RefPtr<Graphics::GraphicsPipelineState>& pipelineState)
        {
            graphicsPipelineStates[key] = pipelineState;
        }
    private:
        Vector<PrimitiveAttribute> attributes;
        Vector<VertexBuffer> vertexBuffers;
        Optional<IndexBuffer> indexBuffer;
        RefPtr<PbrMaterial> material;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        Optional<Rect3f> aabb;
        mutable UnorderedMap<String, RefPtr<Graphics::GraphicsPipelineState>> graphicsPipelineStates;
    };
}
