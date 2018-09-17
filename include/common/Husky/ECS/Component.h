#pragma once

#include <Husky/ECS/Forwards.h>

namespace Husky::ECS
{
    class Component
    {
    public:
        ComponentTypeId GetTypeId() const { return typeId; }
    protected:
        ComponentTypeId typeId;
    };
}
