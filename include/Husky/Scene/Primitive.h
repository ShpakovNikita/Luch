#pragma once

#include <Husky/PrimitiveTopology.h>
#include <Husky/SharedPtr.h>
#include <Husky/Types.h>

namespace Husky::Scene
{
    class Primitive
    {
    public:
        inline PrimitiveTopology GetTopology() const { return topology; }
    private:
        PrimitiveTopology topology;

        SharedPtr<Vector<uint8>> indices;
    };
}
