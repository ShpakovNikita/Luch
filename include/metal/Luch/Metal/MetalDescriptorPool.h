#pragma once

#include <Luch/Graphics/DescriptorPool.h>
#include <Luch/Graphics/DescriptorPoolCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalDescriptorPool : public DescriptorPool
    {
    public:
        MetalDescriptorPool(
            MetalGraphicsDevice* device,
            const DescriptorPoolCreateInfo& aCreateInfo);

        GraphicsResultRefPtr<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout* layout) override;
    private:
        DescriptorPoolCreateInfo createInfo;
    };
}
