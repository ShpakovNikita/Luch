#pragma once

#include <glm/vec3.hpp>
#include <Husky/Types.h>
#include <Husky/MeshGeometry.h>

namespace Husky
{
    class GeometryGenerator
    {
    public:
        static MeshData CreateBox(const String& name, Vec3 position, float32 width, float32 height, float32 depth, int32 materialIndex);
        //static MeshData CreateCylinder(Vec3 bottomCenter, float32 height, float32 topRadius, float32 bottomRadius);
        //static MeshData CreateSphere(Vec3 center, float32 radius);
        //static MeshData CreateGeosphere(Vec3 center, float32 radius);
        //static MeshData CreateCone(Vec3 bottomCenter, float32 height, float32 radius);
        //static MeshData CreateTetrahedron(Vec3 center, float32 side);
    };
}
