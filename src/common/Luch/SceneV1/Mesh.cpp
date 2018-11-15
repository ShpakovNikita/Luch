#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/DescriptorSet.h>

namespace Luch::SceneV1
{
    Mesh::Mesh(RefPtrVector<Primitive>&& aPrimitives, const String &aName)
        : primitives(aPrimitives)
        , name(aName)
    {
    }

    Mesh::~Mesh() = default;
}
