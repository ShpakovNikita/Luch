#include <Luch/ECS/Entity.h>
#include <Luch/ECS/Component.h>
#include <Luch/ECS/System.h>
#include <Luch/Assert.h>
#include <algorithm>

namespace Luch::ECS
{
    void Entity::AddComponent(UniquePtr<Component> component)
    {
        LUCH_ASSERT(component != nullptr);

        auto comp = component.get();
        components[component->GetTypeId()].emplace_back(move(component));

        OnComponentAdded(comp);
        for (auto system : systems)
        {
            system->AddComponent(this, comp);
        }
    }

    bool Entity::RemoveComponent(Component *component)
    {
        auto it = components.find(component->GetTypeId());
        if(it != components.end())
        {
            auto& componentsOfType = it->second;
            if(componentsOfType.size() == 1 && componentsOfType.front().get() == component)
            {
                OnComponentRemoved(component);
                componentsOfType.clear();
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    int32 Entity::RemoveAllComponents(ComponentTypeId type)
    {
        auto it = components.find(type);
        if(it != components.end())
        {
            auto& componentsOfType = it->second;
            int32 count = componentsOfType.size();
            for (auto& component : componentsOfType)
            {
                OnComponentRemoved(component.get());
            }
            componentsOfType.clear();
            return count;
        }
        else
        {
            return 0;
        }
    }

    Component* Entity::GetComponent(ComponentTypeId type, int32 index)
    {
        auto it = components.find(type);
        if(it != components.end())
        {
            auto& componentsOfType = it->second;
            if(componentsOfType.size() == 1)
            {
                return componentsOfType.front().get();
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    int32 Entity::GetComponentCount(ComponentTypeId type) const
    {
        auto it = components.find(type);
        if(it != components.end())
        {
            auto& componentsOfType = it->second;
            return componentsOfType.size();
        }
        else
        {
            return 0;
        }
    }

    void Entity::RegisterToSystem(System* system)
    {
        systems.push_back(system);
        system->AddEntity(this);
    }

    void Entity::UnregisterFromSystem(System *system)
    {
        auto it = std::find(systems.begin(), systems.end(), system);
        if(it != systems.end())
        {
            (*it)->RemoveEntity(this);
            systems.erase(it);
        }
    }

    void Entity::OnComponentAdded(Component* component)
    {
        for (auto system : systems)
        {
            system->AddComponent(this, component);
        }
    }

    void Entity::OnComponentRemoved(Component* component)
    {
        for (auto system : systems)
        {
            system->RemoveComponent(this, component);
        }
    }
}
