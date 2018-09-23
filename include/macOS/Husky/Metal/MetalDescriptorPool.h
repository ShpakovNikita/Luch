#pragma once

#include <Husky/Graphics/DescriptorPool.h>
#include <Husky/Graphics/DescriptorPoolCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
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
