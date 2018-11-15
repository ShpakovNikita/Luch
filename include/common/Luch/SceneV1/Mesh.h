#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>

namespace Luch::SceneV1
{
    class Mesh : public BaseObject
    {
    public:
        Mesh(
            RefPtrVector<Primitive>&& primitives,
            const String& name = "");

        ~Mesh();

        inline const String& GetName() const { return name; }
        inline const RefPtrVector<Primitive>& GetPrimitives() const { return primitives; }

        inline const RefPtr<Graphics::DescriptorSet>& GetBufferDescriptorSet(const String& key) const
        {
            return descriptorSets[key];
        }

        inline void SetBufferDescriptorSet(const String& key, const RefPtr<Graphics::DescriptorSet>& descriptorSet)
        {
            descriptorSets[key] = descriptorSet;
        }
    private:
        RefPtrVector<Primitive> primitives;
        String name;

        mutable UnorderedMap<String, RefPtr<Graphics::DescriptorSet>> descriptorSets;
    };
}
