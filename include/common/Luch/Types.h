#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
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
#include <stack>

namespace Luch
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

template<typename T, typename Less = std::less<T>>
using Set = std::set<T, Less>;

template<typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
using UnorderedSet = std::unordered_set<T, Hash, Equal>;

template<typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
using UnorderedMultiset = std::unordered_multiset<T, Hash, Equal>;

template<typename Key, typename Value>
using Map = std::map<Key, Value>;

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using UnorderedMap = std::unordered_map<Key, Value, Hash, KeyEqual>;

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using UnorderedMultimap = std::unordered_multimap<Key, Value, Hash, KeyEqual>;

template<typename T>
using Stack = std::stack<T>;

using Byte = std::byte;

template<typename T>
using Limits = std::numeric_limits<T>;

template<typename T>
using Optional = std::optional<T>;

template<typename T1, typename T2>
using Pair = std::pair<T1, T2>;

template<typename ... Args>
using Tuple = std::tuple<Args...>;

template<typename ... Args>
using Variant = std::variant<Args...>;

using FilePath = String;//std::experimental::filesystem::path;

}
