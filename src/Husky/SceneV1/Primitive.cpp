#include <Husky/SceneV1/Primitive.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/SceneV1/VertexBuffer.h>

namespace Husky::SceneV1
{
    Primitive::Primitive(
        Vector<PrimitiveAttribute>&& aAttributes,
        RefPtrVector<VertexBuffer>&& aVertexBuffers,
        const RefPtr<IndexBuffer>& aIndexBuffer,
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
}