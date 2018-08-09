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

        inline const RefPtr<Vulkan::DeviceBuffer>& GetUniformBuffer() const { return uniformBuffer; }
        inline void SetUniformBuffer(const RefPtr<Vulkan::DeviceBuffer>& aUniformBuffer) { uniformBuffer = aUniformBuffer; }

        inline const RefPtr<Vulkan::DescriptorSet>& GetDescriptorSet() const { return descriptorSet; }
        inline void SetDescriptorSet(const RefPtr<Vulkan::DescriptorSet>& aDescriptorSet) { descriptorSet = aDescriptorSet; }
    private:
        String name;

        RefPtrVector<Primitive> primitives;

        RefPtr<Vulkan::DeviceBuffer> uniformBuffer;
        RefPtr<Vulkan::DescriptorSet> descriptorSet;
    };
}
