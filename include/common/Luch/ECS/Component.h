#pragma once

#include <Luch/ECS/ECSForwards.h>

namespace Luch::ECS
{
    class Component
    {
    public:
        ComponentTypeId GetTypeId() const { return typeId; }
    protected:
        ComponentTypeId typeId;
    };
}
