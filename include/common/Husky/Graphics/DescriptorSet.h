#pragma once

#include <Husky/Graphics/GraphicsObject.h>
#include <Husky/Graphics/DescriptorSetType.h>

namespace Husky::Graphics
{
    class DescriptorSet : public GraphicsObject
    {
    public:
        DescriptorSet(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~DescriptorSet() = 0;

        virtual DescriptorSetType GetType() const = 0;
    };

    inline DescriptorSet::~DescriptorSet() {}
}
