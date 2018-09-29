#pragma once

#include <Husky/ECS/ECSForwards.h>

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
