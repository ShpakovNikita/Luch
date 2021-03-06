#include <Luch/Metal/MetalError.h>
#include <Luch/Types.h>

namespace Luch::Metal
{
    GraphicsResult LibraryErrorToGraphicsResult(ns::Error error)
    {
        if(error.GetCode() == 0)
        {
            return GraphicsResult::Success;
        }

        switch(error.GetCode())
        {
        case (uint32)mtlpp::LibraryError::CompileWarning:
            return GraphicsResult::CompilerWarning;
        case (uint32)mtlpp::LibraryError::CompileFailure:
            return GraphicsResult::CompilerError;
        case (uint32)mtlpp::LibraryError::Unsupported:
            return GraphicsResult::Unsupported;
        case (uint32)mtlpp::LibraryError::FunctionNotFound:
            return GraphicsResult::FunctionNotFound;
        case (uint32)mtlpp::LibraryError::Internal:
            return GraphicsResult::InternalError;
        default:
            return GraphicsResult::UnknownError;
        }
    }

    GraphicsResult PipelineErrorToGraphicsResult(ns::Error error)
    {
        if(error.GetCode() == 0)
        {
            return GraphicsResult::Success;
        }

        switch(error.GetCode())
        {
        case (uint32)mtlpp::RenderPipelineError::Internal:
            return GraphicsResult::InternalError;
        case (uint32)mtlpp::RenderPipelineError::InvalidInput:
            return GraphicsResult::InvalidValue;
        case (uint32)mtlpp::RenderPipelineError::Unsupported:
            return GraphicsResult::Unsupported;
        default:
            return GraphicsResult::UnknownError;
        }
    }
}
