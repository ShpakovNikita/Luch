#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/PrimitiveTopology.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/AttributeSemantic.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/VertexBuffer.h>

namespace Husky::SceneV1
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
    public:
        Primitive(
            Vector<PrimitiveAttribute>&& attributes,
            Vector<VertexBuffer>&& vertexBuffers,
            const Optional<IndexBuffer>& indexBuffer,
            const RefPtr<PbrMaterial>& material,
            PrimitiveTopology topology
        );

        ~Primitive();

        const Vector<PrimitiveAttribute>& GetAttributes() const { return attributes; }
        const Vector<VertexBuffer>& GetVertexBuffers() const { return vertexBuffers; }
        const Optional<IndexBuffer>& GetIndexBuffer() const { return indexBuffer; }
        const RefPtr<PbrMaterial>& GetMaterial() const { return material; }
        inline PrimitiveTopology GetTopology() const { return topology; }
        inline const RefPtr<Graphics::PipelineState>& GetPipelineState() const { return pipelineState; }
        inline void SetPipelineState(const RefPtr<Graphics::PipelineState>& aPipelineState) { pipelineState = aPipelineState; }
        inline void AddShaderProgram(const RefPtr<Graphics::ShaderProgram>& shaderProgram) { shaderPrograms.push_back(shaderProgram); }
    private:
        Vector<PrimitiveAttribute> attributes;
        Vector<VertexBuffer> vertexBuffers;
        Optional<IndexBuffer> indexBuffer;
        RefPtr<PbrMaterial> material;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        RefPtrVector<Graphics::ShaderProgram> shaderPrograms;
        RefPtr<Graphics::PipelineState> pipelineState;
    };
}
