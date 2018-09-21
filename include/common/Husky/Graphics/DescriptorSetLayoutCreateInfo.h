#pragma once


#include <Husky/Graphics/ShaderStage.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/DscriptorSetBinding.h>

namespace Husky::Graphics
{
    struct DescriptorSetBindingCollection
    {
        UnorderedMap<ShaderStage, Vector<DescriptorSetBinding*>> bindings;
        int32 currentBindingIndex = 0;

        inline void AddBinding(ShaderStage stage, const DescriptorSetBinding& binding)
        {
            binding->index = currentBindingIndex;
            currentBindingIndex += binding->count;
            bindings.push_back(binding);
        }
    };

    class DescriptorSetLayoutCreateInfo
    {
    public:
        inline DescriptorSetLayoutCreateInfo& AddBufferBinding(ShaderStage stage, DescriptorSetBinding* binding)
        {
           bufferBindings.AddBinding(stage, binding);
           return *this;
        }

        inline DescriptorSetLayoutCreateInfo& AddTextureBinding(ShaderStage stage, DescriptorSetBinding* binding)
        {
           textureBindings.AddBinding(stage, binding);
           return *this;
        }

        inline DescriptorSetLayoutCreateInfo& AddSamplerBinding(ShaderStage stage, DescriptorSetBinding* binding)
        {
           samplerBindings.AddBinding(stage, binding);
           return *this;
        }
    private:
        DescriptorSetBindingCollection bufferBindings;
        DescriptorSetBindingCollection textureBindings;
        DescriptorSetBindingCollection samplerBindings;
    };
}
