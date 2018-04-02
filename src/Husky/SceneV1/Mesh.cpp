#include <Husky/SceneV1/Mesh.h>
#include <Husky/SceneV1/Primitive.h>

namespace Husky::SceneV1
{
    Mesh::Mesh(RefPtrVector<Primitive>&& aPrimitives, const String &aName)
        : primitives(aPrimitives)
        , name(aName)
    {
    }

    Mesh::~Mesh() = default;
}
