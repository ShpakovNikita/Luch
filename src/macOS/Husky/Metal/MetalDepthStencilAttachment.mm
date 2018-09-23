#include <Husky/Metal/MetalDepthStencilAttachment.h>
#include <Husky/Assert.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::StencilOperation ToMetalStencilOperation(StencilOperation operation)
    {
        switch(operation)
        {
        case StencilOperation::Keep:
            return mtlpp::StencilOperation::Keep;
        case StencilOperation::Zero:
            return mtlpp::StencilOperation::Zero;
        case StencilOperation::Replace:
            return mtlpp::StencilOperation::Replace;
        case StencilOperation::IncrementAndClamp:
            return mtlpp::StencilOperation::IncrementClamp;
        case StencilOperation::DecrementAndClamp:
            return mtlpp::StencilOperation::DecrementClamp;
        case StencilOperation::IncrementAndWrap:
            return mtlpp::StencilOperation::IncrementWrap;
        case StencilOperation::DecrementAndWrap:
            return mtlpp::StencilOperation::DecrementWrap;
        case StencilOperation::Invert:
            return mtlpp::StencilOperation::Invert;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown stencil operation");
            return mtlpp::StencilOperation::Zero;
        }
    }
}
