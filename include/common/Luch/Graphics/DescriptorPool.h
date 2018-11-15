#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class DescriptorPool : public GraphicsObject
    {
    public:
        DescriptorPool(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~DescriptorPool() = 0;

        virtual GraphicsResultRefPtr<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout* layout) = 0;
    };

    inline DescriptorPool::~DescriptorPool() {}
}
