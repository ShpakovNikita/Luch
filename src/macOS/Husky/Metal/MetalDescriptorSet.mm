#include <Husky/Metal/MetalDescriptorSet.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalDescriptorSet::MetalDescriptorSet(
        MetalGraphicsDevice* device,
        DescriptorSetType aType,
        int32 aStart)
        : DescriptorSet(device)
        , type(aType)
        , start(aStart)
    {
    }
}
