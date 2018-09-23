#pragma once

#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/BufferCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalBuffer : public Buffer
    {
        friend class MetalGraphicsCommandList;
    public:
        MetalBuffer(
            MetalGraphicsDevice* device,
            const BufferCreateInfo& createInfo,
            mtlpp::Buffer buffer);

        const BufferCreateInfo& GetCreateInfo() const { return createInfo; }
    private:
        BufferCreateInfo createInfo;
        mtlpp::Buffer buffer;
    };
}
