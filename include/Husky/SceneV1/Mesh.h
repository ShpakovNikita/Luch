#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

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

        inline const RefPtr<Vulkan::Buffer>& GetUniformBuffer() const { return uniformBuffer; }
        inline void SetUniformBuffer(const RefPtr<Vulkan::Buffer>& aUniformBuffer) { uniformBuffer = aUniformBuffer; }
    private:
        String name;

        RefPtrVector<Primitive> primitives;

        RefPtr<Vulkan::Buffer> uniformBuffer;
    };
}
