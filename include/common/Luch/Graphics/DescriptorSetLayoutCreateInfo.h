#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/DescriptorSetType.h>
#include <Luch/Graphics/DescriptorSetBinding.h>

namespace Luch::Graphics
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
            LUCH_ASSERT(binding->index < 0);
            binding->index = currentBindingIndex;
            currentBindingIndex++;
            bindings.push_back(*binding);
            return *this;
        }

        DescriptorSetType type = DescriptorSetType::Unknown;
        int32 currentBindingIndex = 0;
        Vector<DescriptorSetBinding> bindings;
    };
}
