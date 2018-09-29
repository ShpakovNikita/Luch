#pragma once

#include <cmath>

namespace Husky::Math
{

template<typename T>
T Clamp(T value, T min, T max)
{
    return std::min(max, std::max(value, min));
}

}
