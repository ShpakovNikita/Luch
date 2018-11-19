#include <Luch/Metal/MetalDepthStencilAttachment.h>
#include <Luch/Assert.h>

namespace Luch::Metal
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
            LUCH_ASSERT_MSG(false, "Unknown stencil operation");
            return mtlpp::StencilOperation::Zero;
        }
    }

    mtlpp::CompareFunction ToMetalCompareFunction(CompareFunction compareFunction)
    {
        switch(compareFunction)
        {
        case CompareFunction::Never:
            return mtlpp::CompareFunction::Never;
        case CompareFunction::Less:
            return mtlpp::CompareFunction::Less;
        case CompareFunction::Equal:
            return mtlpp::CompareFunction::LessEqual;
        case CompareFunction::LessEqual:
            return mtlpp::CompareFunction::LessEqual;
        case CompareFunction::Greater:
            return mtlpp::CompareFunction::Greater;
        case CompareFunction::NotEqual:
            return mtlpp::CompareFunction::NotEqual;
        case CompareFunction::GreaterEqual:
            return mtlpp::CompareFunction::GreaterEqual;
        case CompareFunction::Always:
            return mtlpp::CompareFunction::Always;
        default:
            LUCH_ASSERT_MSG(false, "Unknown compare function");
            return mtlpp::CompareFunction::Never;
        }
    }
}
