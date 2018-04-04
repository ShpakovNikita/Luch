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

        RefPtr& operator==(RefPtr&& other)
        {
            ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
        }

        friend bool operator==(const RefPtr& ptr, nullptr_t np)
        {
            return ptr.ptr == np;
        }

        friend bool operator==(nullptr_t np, const RefPtr& ptr)
        {
            return ptr.ptr == np;
        }

        friend bool operator!=(const RefPtr& ptr, nullptr_t np)
        {
            return ptr.ptr != np;
        }

        friend bool operator!=(nullptr_t np, const RefPtr& ptr)
        {
            return ptr.ptr != np;
        }

        operator bool()
        {
            return ptr != nullptr;
        }

        bool operator==(nullptr_t other)
        {
            return ptr == nullptr;
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
    using RefPtrVector = Vector<RefPtr<T>>;

    template<typename T>
    using RefPtrSet = Set<RefPtr<T>>;
}


