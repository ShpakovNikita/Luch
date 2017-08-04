#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Husky
{

using int8 = ::int8_t;
using int16 = ::int16_t;
using int32 = ::int32_t;
using int64 = ::int64_t;

using uint8 = ::uint8_t;
using uint16 = ::uint16_t;
using uint32 = ::uint32_t;
using uint64 = ::uint64_t;

using float32 = float;
using float64 = double;

using String = std::string;
using StringView = std::string_view;

template<typename T, int N>
using Array = std::array<T, N>;

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using Limits = std::numeric_limits<T>;

}
