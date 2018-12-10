#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Attachment.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::LoadAction ToMetalLoadAction(AttachmentLoadOperation loadOperation);
    mtlpp::StoreAction ToMetalStoreAction(AttachmentStoreOperation storeOperation);
    mtlpp::RenderPassDescriptor ToMetalRenderPassDescriptor(const RenderPassCreateInfo& createInfo);
}
