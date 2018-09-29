#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DescriptorSetType.h>
#include <Husky/Graphics/DescriptorSetBinding.h>

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

        inline DescriptorSetLayoutCreateInfo& WithNBindings(int32 count)
        {
            bindings.reserve(count);
            return *this;
        }

        inline DescriptorSetLayoutCreateInfo& AddBinding(DescriptorSetBinding* binding)
        {
            binding->index = currentBindingIndex;
            currentBindingIndex++;
            bindings.push_back(binding);
            return *this;
        }

        DescriptorSetType type = DescriptorSetType::Unknown;
        int32 currentBindingIndex = 0;
        Vector<DescriptorSetBinding*> bindings;
    };
}
