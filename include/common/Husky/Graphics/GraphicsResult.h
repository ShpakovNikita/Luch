#pragma once

namespace Husky::Graphics
{
    enum class GraphicsResult
    {
        Success,

        UnknownError,
        Unsupported,

        CompilerError,
        CompilerWarning,
        FunctionNotFound,
    };
}
