#pragma once

#include <memory>

namespace Luch
{

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template< class T, class... Args >
inline UniquePtr<T> MakeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

}
