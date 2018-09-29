#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/Attachment.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::LoadAction ToMetalLoadAction(AttachmentLoadOperation loadOperation);
    mtlpp::StoreAction ToMetalStoreAction(AttachmentStoreOperation storeOperation);
    mtlpp::RenderPassDescriptor ToMetalRenderPasDescriptor(const RenderPassCreateInfo& createInfo);
}
