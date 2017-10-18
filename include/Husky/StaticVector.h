#pragma once

#include <Husky/Types.h>

namespace Husky
{
    template<typename T, int32 MaxSize>
    class static_vector
    {
        using array_type = Array<T, MaxSize>;
    public:
        using value_type = array_type::value_type;
        using size_type = array_type::size_type;
        using difference_type = array_type::difference_type;
        using pointer = array_type::pointer;
        using const_pointer = array_type::const_pointer;
        using reference = array_type::reference;
        using const_reference = array_type::const_reference;

        using iterator = array_type::iterator;
        using const_iterator = array_type::const_iterator;

        using reverse_iterator = array_type::reverse_iterator;
        using const_reverse_iterator = array_type::const_reverse_iterator;

        constexpr size_type max_size() const { return MaxSize; }
        constexpr size_type capacity() const { return max_size(); }
        size_type size() const { return end - array.begin(); }

        inline void push_back(const T& value) { *(end++) = value; }
        inline void push_back(T&& value) { *(end++) = value; }

        inline void pop_back()

        template<class... Args>
        reference emplace_back(Args&&... args)
        {
            auto ptr = &(*(end++));
            new (ptr) T{ std::forward<Args>(args)... };
            return *ptr;
        }

    private:
        array_type array;
        iterator end = array.end();
    };
}
