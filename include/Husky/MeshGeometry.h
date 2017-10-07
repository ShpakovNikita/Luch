#pragma once

#include <memory>
#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/Vertex.h>

namespace Husky
{
    struct MeshData
    {
        vk::PrimitiveTopology primitiveTopology;
        String name;
        Vector<int16> indices16;
        Vector<int32> indices32;
        std::shared_ptr<Vector<Vertex>> vertices;
    };

    struct MeshGeometry
    {
        ByteArray hostVertexBuffer;
        ByteArray indexVertexBuffer;
    };
}
