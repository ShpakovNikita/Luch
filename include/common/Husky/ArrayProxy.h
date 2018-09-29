#pragma once

#include <Husky/Types.h>

namespace Husky
{
    template <typename T>
    class ArrayProxy
    {
    public:
    ArrayProxy(std::nullptr_t)
        : count(0)
        , ptr(nullptr)
    {}

    ArrayProxy(T& value)
        : count(1)
        , ptr(&value)
    {}

    ArrayProxy(int32 aCount, T* aPtr)
        : count(aCount)
        , ptr(aPtr)
    {}

    template <size_t N>
    ArrayProxy(std::array<typename std::remove_const<T>::type, N>& data)
        : count(N)
        , ptr(data.data())
    {}

    template <size_t N>
    ArrayProxy(std::array<typename std::remove_const<T>::type, N> const& data)
        : count(N)
        , ptr(data.data())
    {}

    template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
    ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator>& data)
        : count(static_cast<int32>(data.size()))
        , ptr(data.data())
    {}

    template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
    ArrayProxy(const std::vector<typename std::remove_const<T>::type, Allocator>& data)
        : count(static_cast<int32>(data.size()))
        , ptr(data.data())
    {}

    ArrayProxy(const std::initializer_list<T>& data)
        : count(static_cast<int32>(data.end() - data.begin()))
        , ptr(data.begin())
    {}

    /*template<typename P>
    ArrayProxy(const std::initializer_list<P* const>& data)
        : count(static_cast<int32>(data.end() - data.begin()))
        , ptr(data.begin())
    {}*/

    const T* begin() const
    {
        return ptr;
    }

    const T* end() const
    {
        return ptr + count;
    }

    const T& front() const
    {
        assert(count && ptr);
        return *ptr;
    }

    const T& back() const
    {
        assert(count && ptr);
        return *(ptr + count - 1);
    }

    bool empty() const
    {
        return (count == 0);
    }

    int32 size() const
    {
        return count;
    }

    const T* data() const
    {
        return ptr;
    }

    private:
        int32 count;
        const T* ptr;
    };

    template <typename T>
    class PtrArrayProxy;

    template<typename T>
    class PtrArrayProxy<T*> : public ArrayProxy<T* const>
    {
    };
}
