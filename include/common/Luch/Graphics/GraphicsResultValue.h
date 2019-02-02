#pragma once

#include <Luch/ResultValue.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsResult.h>

namespace Luch::Graphics
{
    template<typename Value>
    using GraphicsResultValue = ResultValue<GraphicsResult, Value>;

    template<typename Value>
    using GraphicsResultRefPtr = GraphicsResultValue<RefPtr<Value>>;
}
