#pragma once

#include <Luch/ECS/ECSForwards.h>

namespace Luch::ECS
{
    class System
    {
    public:
        inline virtual void AddEntity(Entity*) {};
        inline virtual void RemoveEntity(Entity*) {};
        inline virtual void AddComponent(Entity*, Component*) {};
        inline virtual void RemoveComponent(Entity*, Component*) {};
    };
}
