#pragma once

#include <Husky/Graphics/GraphicsObject.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class DescriptorPool : public GraphicsObject
    {
    public:
        DescriptorPool(GraphicsDevice* deivce) : GraphicsObject(device) {}
        virtual ~DescriptorPool() = 0 {};

        virtual GraphicsResultRefPtr<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout* layout) = 0;
    };
}
