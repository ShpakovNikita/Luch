#include <Husky/Metal/MetalError.h>
#include <Husky/Types.h>

namespace Husky::Metal
{
    GraphicsResult ToGraphicsResult(ns::Error error)
    {
        if(!error)
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
        default:
            return GraphicsResult::UnknownError;
        }
    }
}
