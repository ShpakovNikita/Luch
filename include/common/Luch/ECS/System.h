#pragma once

#include <Luch/ECS/ECSForwards.h>

namespace Luch::ECS
{
    class System
    {
    public:
        inline virtual void AddEntity(Entity* entity) {};
        inline virtual void RemoveEntity(Entity* entity) {};
        inline virtual void AddComponent(Entity* entity, Component* component) {};
        inline virtual void RemoveComponent(Entity* entity, Component* component) {};
    };
}
