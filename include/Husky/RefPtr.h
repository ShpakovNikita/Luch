#pragma once

#include <Husky/Types.h>
#include <Husky/BaseObject.h>

namespace Husky
{
    template<typename T>
    class RefPtr final
    {
        template<typename U>
        friend class RefPtr;
    public:
        RefPtr() = default;

        RefPtr(T* aPtr)
            : ptr(aPtr)
        {
        }

        ~RefPtr()
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->RemoveReference();
            }
        }

        RefPtr(const RefPtr& other)
            : ptr(other.ptr)
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
        }

        RefPtr(RefPtr&& other)
            : ptr(other.ptr)
        {
            other.ptr = nullptr;
        }

        template<typename U>
        RefPtr(RefPtr<U>&& other)
            : ptr(static_cast<T*>(other.ptr))
        {
            other.ptr = nullptr;
        }

        template<typename U>
        RefPtr(const RefPtr<U>& other)
            : ptr(static_cast<T*>(other.ptr))
        {
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
        }

        RefPtr& operator=(const RefPtr& other)
        {
            ptr = other.ptr;
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
            return *this;
        }

        template<typename U>
        RefPtr& operator=(const RefPtr<U>& other)
        {
            ptr = static_cast<T*>(other.ptr);
            if (ptr)
            {
                reinterpret_cast<BaseObject*>(ptr)->AddReference();
            }
            return *this;
        }

        RefPtr& operator=(RefPtr&& other)
        {
            ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
        }

        bool operator==(const RefPtr& right) const
        {
            return ptr == right.ptr;
        }

        //friend bool operator!=(const RefPtr& left, RefPtr& right)
        //{
        //    return left.ptr != right.ptr;
        //}

        bool operator!=(RefPtr& right) const
        {
            return ptr != right.ptr;
        }

        bool operator<(const RefPtr& right) const
        {
            return ptr < right.ptr;
        }

        bool operator==(std::nullptr_t np) const
        {
            return ptr == np;
        }

        friend bool operator==(std::nullptr_t np, const RefPtr& ptr)
        {
            return ptr.ptr == np;
        }

        bool operator!=(std::nullptr_t np) const
        {
            return ptr != np;
        }

        friend bool operator!=(std::nullptr_t np, const RefPtr& ptr)
        {
            return ptr.ptr != np;
        }

        operator T*() const
        {
            return ptr;
        }

        operator bool() const
        {
            return ptr != nullptr;
        }

        T* operator->() const
        {
            return ptr;
        }

        T* Get() const
        {
            return ptr;
        }

        T* Release()
        {
            return std::move(*this).Get();
        }

        explicit operator T*()
        {
            return ptr;
        }
    private:
        T * ptr = nullptr;
    };

    template<typename T, typename... Args>
    inline RefPtr<T> MakeRef(Args&&... args)
    {
        return RefPtr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename T>
    class RefPtrEqualTo
    {
    public:
        bool operator()(const RefPtr<T>& left, const RefPtr<T>& right) const
        {
            return left.operator==(right);
        }
    };

    template<typename T>
    class RefPtrLess
    {
    public:
        bool operator()(const RefPtr<T>& left, const RefPtr<T>& right) const
        {
            return left.operator<(right);
        }
    };

    template<typename T>
    class RefPtrHash
    {
    public:
        bool operator()(const RefPtr<T>& ptr) const
        {
            return std::hash<T*>{}(ptr);
        }
    };

    template<typename T>
    using RefPtrVector = Vector<RefPtr<T>>;

    //template<typename T>
    //using RefPtrVectorSet = VectorSet<RefPtr<T>>;

    template<typename T>
    using RefPtrSet = Set<RefPtr<T>, RefPtrLess<T>>;

    template<typename T>
    using RefPtrUnorderedSet = UnorderedSet<RefPtr<T>, RefPtrHash<T>>;
}
