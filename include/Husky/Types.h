#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
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

using uchar8 = unsigned char;
using char8 = char;

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

template<typename TKey, typename TValue>
using Map = std::map<TKey, TValue>;

template<typename TKey, typename TValue>
using UnorderedMap = std::unordered_map<TKey, TValue>;

}
