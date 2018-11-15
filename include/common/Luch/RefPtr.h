#pragma once

#include <Luch/Types.h>
#include <Luch/BaseObject.h>

namespace Luch
{
    template<typename T>
    class RefPtr final
    {
        template<typename U>
        friend class RefPtr;
    public:
        RefPtr() noexcept = default;

        explicit RefPtr(T* aPtr) noexcept
            : ptr(aPtr)
        {
        }

        RefPtr(std::nullptr_t nptr) noexcept
            : ptr(nullptr)
        {
        }

        ~RefPtr()
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->RemoveReference();
            }
        }

        RefPtr(const RefPtr& other) noexcept
            : ptr(other.ptr)
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
        }

        RefPtr(RefPtr&& other) noexcept
            : ptr(other.ptr)
        {
            other.ptr = nullptr;
        }

        template<typename U>
        RefPtr(RefPtr<U>&& other) noexcept
            : ptr(static_cast<T*>(other.ptr))
        {
            other.ptr = nullptr;
        }

        template<typename U>
        RefPtr(const RefPtr<U>& other) noexcept
            : ptr(static_cast<T*>(other.ptr))
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
        }

        RefPtr& operator=(const RefPtr& other) noexcept
        {
            ptr = other.ptr;
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
            return *this;
        }

        template<typename U>
        RefPtr& operator=(const RefPtr<U>& other) noexcept
        {
            ptr = static_cast<T*>(other.ptr);
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
            return *this;
        }

        RefPtr& operator=(RefPtr&& other) noexcept
        {
            ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
        }

        friend bool operator==(const RefPtr& left, const RefPtr& right) noexcept
        {
            return left.ptr == right.ptr;
        }

        friend bool operator!=(const RefPtr& left, const RefPtr& right) noexcept
        {
            return left.ptr != right.ptr;
        }

        friend bool operator<(const RefPtr& left, const RefPtr& right) noexcept
        {
            return left.ptr < right.ptr;
        }

        friend bool operator==(const RefPtr& ptr, std::nullptr_t np) noexcept
        {
            return ptr.ptr == np;
        }

        friend bool operator==(std::nullptr_t np, const RefPtr& ptr) noexcept
        {
            return ptr.ptr == np;
        }

        friend bool operator!=(std::nullptr_t np, const RefPtr& ptr) noexcept
        {
            return ptr.ptr != np;
        }

        friend bool operator!=(const RefPtr& ptr, std::nullptr_t np) noexcept
        {
            return ptr.ptr != np;
        }

        operator T*() const noexcept
        {
            return ptr;
        }

        operator bool() const noexcept
        {
            return ptr != nullptr;
        }

        T* operator->() const noexcept
        {
            return ptr;
        }

        T* Get() const noexcept
        {
            return ptr;
        }

        void Release()
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->RemoveReference();
            }
        }

        explicit operator T*() noexcept
        {
            return ptr;
        }
    private:
        T* ptr = nullptr;
    };

    template<typename T, typename... Args>
    [[nodiscard]] inline RefPtr<T> MakeRef(Args&&... args)
    {
        return RefPtr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename T>
    class RefPtrEqualTo
    {
    public:
        bool operator()(const RefPtr<T>& left, const RefPtr<T>& right) const noexcept
        {
            return left.operator==(right);
        }
    };

    template<typename T>
    class RefPtrLess
    {
    public:
        bool operator()(const RefPtr<T>& left, const RefPtr<T>& right) const noexcept
        {
            return left.operator<(right);
        }
    };

    template<typename T>
    class RefPtrHash
    {
    public:
        std::size_t operator()(const RefPtr<T>& ptr) const noexcept
        {
            return std::hash<T*>{}(ptr);
        }
    };

    template<typename T>
    using RefPtrVector = Vector<RefPtr<T>>;

    template<typename T, int32 N>
    using RefPtrArray = Array<RefPtr<T>, N>;

    template<typename T>
    using RefPtrSet = Set<RefPtr<T>, RefPtrLess<T>>;

    template<typename T>
    using RefPtrUnorderedSet = UnorderedSet<RefPtr<T>, RefPtrHash<T>>;
}
