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

        virtual DescriptorSetLayout* GetDescriptorSetLayout() = 0;

        virtual void WriteTexture(DescriptorSetBinding* binding, Texture* texture) = 0;
        virtual void WriteUniformBuffer(DescriptorSetBinding* binding, Buffer* buffer) = 0;
        virtual void WriteSampler(DescriptorSetBinding* binding, Sampler* sampler) = 0;
        virtual void Update() = 0;
    };

    inline DescriptorSet::~DescriptorSet() {}
}
