#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetType.h>

namespace Husky::Graphics
{
    class DescriptorSetLayoutCreateInfo
    {
    public:
        inline DescriptorSetLayoutCreateInfo& OfType(DescriptorSetType aType)
        {
            type = aType;
            return *this;
        }

        inline DescriptorSetLayoutCreateInfo& AddBinding(DescriptorSetBinding* binding)
        {
            binding->index = currentBindingIndex;
            currentBindingIndex++;
            bindings.push_back(binding);
            return *this;
        }
    private:
        DescriptorSetType type;
        int32 currentBindingIndex = 0;
        Vector<DescriptorSetBinding*> bindings;
    };
}
