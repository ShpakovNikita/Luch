#pragma once

namespace Luch::Graphics
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
