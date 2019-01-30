#pragma once

#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/DescriptorSetType.h>

namespace Luch::Graphics
{
    class DescriptorSet : public GraphicsObject
    {
    public:
        DescriptorSet(GraphicsDevice* device) : GraphicsObject(device) {}
        virtual ~DescriptorSet() = 0;

        virtual DescriptorSetLayout* GetDescriptorSetLayout() = 0;

        virtual void WriteTexture(const DescriptorSetBinding& binding, Texture* texture) = 0;
        virtual void WriteUniformBuffer(const DescriptorSetBinding& binding, Buffer* buffer, int32 offset) = 0;
        virtual void WriteThreadgroupMemory(const DescriptorSetBinding& binding, int32 length, int32 offset) = 0;
        virtual void WriteSampler(const DescriptorSetBinding& binding, Sampler* sampler) = 0;
        virtual void Update() = 0;
    };

    inline DescriptorSet::~DescriptorSet() {}
}
