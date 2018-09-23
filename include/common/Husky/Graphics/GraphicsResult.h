#pragma once

namespace Husky::Graphics
{
    enum class GraphicsResult
    {
        Success,

        InternalError,

        UnknownError,
        Unsupported,

        CompilerError,
        CompilerWarning,
        FunctionNotFound,
        InvalidValue,
    };
}
