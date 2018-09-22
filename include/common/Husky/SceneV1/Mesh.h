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

        inline const RefPtr<Graphics::Buffer>& GetUniformBuffer() const { return uniformBuffer; }
        inline void SetUniformBuffer(const RefPtr<Graphics::Buffer>& aUniformBuffer) { uniformBuffer = aUniformBuffer; }

        inline const RefPtr<Graphics::DescriptorSet>& GetDescriptorSet() const { return descriptorSet; }
        inline void SetDescriptorSet(const RefPtr<Graphics::DescriptorSet>& aDescriptorSet) { descriptorSet = aDescriptorSet; }
    private:
        String name;

        RefPtrVector<Primitive> primitives;

        RefPtr<Graphics::Buffer> uniformBuffer;
        RefPtr<Graphics::DescriptorSet> descriptorSet;
    };
}
