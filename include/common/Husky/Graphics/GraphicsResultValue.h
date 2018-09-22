#pragma once

#include <Husky/ResultValue.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsResult.h>

namespace Husky::Graphics
{
    template<typename Value>
    using GraphicsResultValue = ResultValue<GraphicsResult, Value>;

    template<typename T>
    using GraphicsResultRefPtr = GraphicsResultValue<RefPtr<T>>;
}
