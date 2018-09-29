#pragma once

#include <Husky/Graphics/GraphicsObject.h>

namespace Husky::Graphics
{
    class DescriptorSetLayout : public GraphicsObject
    {
    public:
        DescriptorSetLayout(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~DescriptorSetLayout() = 0;

        virtual const DescriptorSetLayoutCreateInfo& GetCreateInfo() const = 0;
    };

    inline DescriptorSetLayout::~DescriptorSetLayout() {}
}
