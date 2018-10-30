#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>

namespace Husky::SceneV1
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
