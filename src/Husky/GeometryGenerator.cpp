#include <Husky/GeometryGenerator.h>

namespace Husky
{
    MeshData GeometryGenerator::CreateBox(
        const String& name,
        Vec3 position,
        float32 width,
        float32 height,
        float32 depth,
        int32 materialIndex)
    {
        // We'll UV map box texture coordinates like this:
        //     (0, 0)----------(0.33, 0)-----------(0.66, 0)----------(1, 0)
        //        |                 |                  |                 |
        //        |      Front      |      Back        |       Top       |
        //        |                 |                  |                 |
        //    (0, 0.5)-------(0.33, 0.5)----------(0.66, 0.5)---------(1, 0.5)
        //        |                 |                  |                 |
        //        |     Bottom      |      Left        |      Right      |
        //        |                 |                  |                 |
        //     (0, 1)-----------(0.33, 1)---------(0.66, 1)-----------(1, 1)

        MeshData boxMeshData;

        boxMeshData.name = name;

        boxMeshData.primitiveTopology = vk::PrimitiveTopology::eTriangleList;

        float32 leftX = position.x;
        float32 rightX = leftX + width;

        float32 topY = position.y;
        float32 bottomY = topY - height;

        float32 backZ = position.z;
        float32 frontZ = backZ + depth;

        Vector<Vertex> vertices =
        {
            // Front
            Vertex{ Vec3{ leftX, topY, frontZ }, Vec3{0, 0, -1}, Vec2{ 0, 0 }, materialIndex },
            Vertex{ Vec3{ rightX, topY, frontZ }, Vec3{ 0, 0, -1 }, Vec2{ 1 / 3.0f, 0 }, materialIndex },
            Vertex{ Vec3{ rightX, bottomY, frontZ }, Vec3{ 0, 0, -1 }, Vec2{ 1 / 3.0f, 0.5f }, materialIndex },
            Vertex{ Vec3{ leftX, bottomY, frontZ }, Vec3{ 0, 0, -1 }, Vec2{ 0, 0.5f }, materialIndex },

            // Back
            Vertex{ Vec3{ rightX, topY, backZ }, Vec3{ 0, 0, 1 }, Vec2{ 1 / 3.0f, 0 }, materialIndex },
            Vertex{ Vec3{ leftX, topY, backZ }, Vec3{ 0, 0, 1 }, Vec2{ 2 / 3.0f, 0 }, materialIndex },
            Vertex{ Vec3{ leftX, bottomY, backZ }, Vec3{ 0, 0, 1 }, Vec2{ 2 / 3.0f, 0.5f }, materialIndex },
            Vertex{ Vec3{ rightX, bottomY, backZ }, Vec3{ 0, 0, 1 }, Vec2{ 1 / 3.0f, 0.5f }, materialIndex },

            // Top
            Vertex{ Vec3{ leftX, topY, backZ }, Vec3{ 0, 1, 0 }, Vec2{ 2 / 3.0f, 0 }, materialIndex },
            Vertex{ Vec3{ rightX, topY, backZ }, Vec3{ 0, 1, 0 }, Vec2{ 1, 0 }, materialIndex },
            Vertex{ Vec3{ rightX, topY, frontZ }, Vec3{ 0, 1, 0 }, Vec2{ 1, 0.5f }, materialIndex },
            Vertex{ Vec3{ leftX, topY, frontZ }, Vec3{ 0, 1, 0 }, Vec2{ 2 / 3.0f, 0.5f }, materialIndex },

            // Bottom
            Vertex{ Vec3{ leftX, bottomY, frontZ }, Vec3{ 0, -1, 0 }, Vec2{ 0, 0.5f }, materialIndex },
            Vertex{ Vec3{ rightX,bottomY, frontZ }, Vec3{ 0, -1, 0 }, Vec2{ 1 / 3.0f, 0.5f }, materialIndex },
            Vertex{ Vec3{ rightX,bottomY, backZ }, Vec3{ 0, -1, 0 }, Vec2{ 1 / 3.0f, 1 }, materialIndex },
            Vertex{ Vec3{ leftX, bottomY, backZ }, Vec3{ 0, -1, 0 }, Vec2{ 0, 1 }, materialIndex },

            // Left
            Vertex{ Vec3{ leftX, topY, backZ }, Vec3{ -1, 0, 0 }, Vec2{ 1 / 3.0f, 0.5f }, materialIndex },
            Vertex{ Vec3{ leftX, topY, frontZ }, Vec3{ -1, 0, 0 }, Vec2{ 2 / 3.0f, 0.5f }, materialIndex },
            Vertex{ Vec3{ leftX, bottomY, frontZ }, Vec3{ -1, 0, 0 }, Vec2{ 2 / 3.0f, 1 }, materialIndex },
            Vertex{ Vec3{ leftX, bottomY, backZ }, Vec3{ -1, 0, 0 }, Vec2{ 1 / 3.0f, 1 }, materialIndex },

            // Right
            Vertex{ Vec3{ rightX, topY, frontZ }, Vec3{ 1, 0, 0 }, Vec2{ 2 / 3.0f, 0.5f }, materialIndex },
            Vertex{ Vec3{ rightX, topY, backZ }, Vec3{ 1, 0, 0 }, Vec2{ 1, 0.5f }, materialIndex },
            Vertex{ Vec3{ rightX, bottomY, backZ }, Vec3{ 1, 0, 0 }, Vec2{ 1, 1 }, materialIndex },
            Vertex{ Vec3{ rightX, bottomY, frontZ }, Vec3{ 1, 0, 0 }, Vec2{ 2 / 3.0f, 1 }, materialIndex },
        };


        boxMeshData.indices16 =
        {
            0, 1, 2,
            2, 3, 0,

            4, 5, 6,
            6, 7, 4,

            8,  9,  10,
            10, 11, 8,

            12, 13, 14,
            14, 15, 12,

            16, 17, 18,
            18, 19, 16,

            20, 21, 22,
            22, 23, 20,
        };

        boxMeshData.vertices = vertices;

        return boxMeshData;
    }

    //MeshData GeometryGenerator::CreateCylinder(vec3 bottomCenter, f32 height, f32 topRadius, f32 bottomRadius) const
    //{
    //    // TODO
    //    return MeshData{};
    //}

    //MeshData GeometryGenerator::CreateSphere(vec3 center, f32 radius) const
    //{
    //    // TODO
    //    return MeshData{};
    //}

    //MeshData GeometryGenerator::CreateGeosphere(vec3 center, f32 radius) const
    //{
    //    // TODO
    //    return MeshData{};
    //}

    //MeshData GeometryGenerator::CreateCone(vec3 bottomCenter, f32 height, f32 radius) const
    //{
    //    // TODO
    //    return MeshData{};
    //}

    //MeshData GeometryGenerator::CreateTetrahedron(vec3 center, f32 side) const
    //{
    //    // TODO
    //    return MeshData{};
    //}

}
