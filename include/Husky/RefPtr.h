#pragma once

#include <Husky/Types.h>

namespace Husky
{
    template<typename T>
    class RefPtr final
    {
    public:
        RefPtr() = default;

        RefPtr(T* aPtr)
            : ptr(aPtr)
        {
        }

        ~RefPtr()
        {
            ptr->RemoveReference();
        }

        RefPtr(const RefPtr& other)
            : ptr(other.ptr)
        {
            ptr->AddReference();
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

        }

        template<typename U>
        RefPtr(const RefPtr<U>& other)
            : ptr(static_cast<T*>(other.ptr))
        {
            ptr->AddReference();
        }

        RefPtr& operator=(const RefPtr& other)
        {
            ptr = other.ptr;
            ptr->AddReference();
        }

        bool operator==(const RefPtr& other)
        {
            return ptr == other.ptr;
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
}
