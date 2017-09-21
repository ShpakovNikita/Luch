#pragma once

#include <type_traits>

#define DEFINE_OPERATORS_FOR_FLAGS_ENUM(T)                                                                                            \
    static_assert(std::is_enum<T>::value, "Not an enum");                                                                             \
    constexpr inline T operator|(T left, T right)                                                                                     \
    {                                                                                                                                 \
        return static_cast<T>(static_cast<std::underlying_type<T>::type>(left) | static_cast<std::underlying_type<T>::type>(right));  \
    }                                                                                                                                 \
    constexpr inline T operator&(T left, T right)                                                                                     \
    {                                                                                                                                 \
        return static_cast<T>(static_cast<std::underlying_type<T>::type>(left) & static_cast<std::underlying_type<T>::type>(right));  \
    }                                                                                                                                 \
    inline T operator|=(T& left, T right)                                                                                             \
    {                                                                                                                                 \
        left = left | right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline T operator&=(T& left, T right)                                                                                             \
    {                                                                                                                                 \
        left = left & right;                                                                                                          \
        return left;                                                                                                                  \
    }                                                                                                                                 \
    inline T operator~(T value)                                                                                                       \
    {                                                                                                                                 \
        return static_cast<T>(~static_cast<std::underlying_type<T>::type>(value));                                                    \
    }                                                                                                                                 \
                                                                                                                                      \

