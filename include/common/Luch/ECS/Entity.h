#pragma once

#include <Luch/UniquePtr.h>
#include <Luch/ECS/ECSForwards.h>

namespace Luch::ECS
{
    class Entity
    {
    public:
        void AddComponent(UniquePtr<Component> component);

        bool RemoveComponent(Component* component);
        int32 RemoveAllComponents(ComponentTypeId type);

        Component* GetComponent(ComponentTypeId type, int32 index = 0);
        int32 GetComponentCount(ComponentTypeId type) const;

        void RegisterToSystem(System* system);
        void UnregisterFromSystem(System* system);
    protected:
        UnorderedMap<ComponentTypeId, Vector<UniquePtr<Component>>> components;
        Vector<System*> systems;
    private:
        void OnComponentAdded(Component* component);
        void OnComponentRemoved(Component* component);
    };
}

