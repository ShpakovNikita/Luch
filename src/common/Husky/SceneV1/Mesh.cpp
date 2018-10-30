#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>
#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/DescriptorSet.h>

namespace Husky::SceneV1
{
    Mesh::Mesh(RefPtrVector<Primitive>&& aPrimitives, const String &aName)
        : primitives(aPrimitives)
        , name(aName)
    {
    }

    Mesh::~Mesh() = default;
}
