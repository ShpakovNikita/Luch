#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <set>
#include <unordered_set>
#include <tuple>
#include <memory>
#include <variant>

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

using char8 = char;
using byte = char8;

template<typename T, int32 Size>
using Array = std::array<T, Size>;

template<typename T>
using Vector = std::vector<T>;

using ByteArray = std::vector<char8>;

template<typename T>
using Set = std::set<T>;

template<typename T>
using UnorderedSet = std::unordered_set<T>;

template<typename T>
using Limits = std::numeric_limits<T>;

template<typename T>
using Optional = std::optional<T>;

template<typename ... Args>
using Tuple = std::tuple<Args...>;

template<typename ... Args>
using Variant = std::variant<Args...>;

}
