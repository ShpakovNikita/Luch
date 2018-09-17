#pragma once

#include <Husky/ECS/Forwards.h>

namespace Husky::ECS
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
