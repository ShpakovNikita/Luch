#pragma once

#include <memory>

namespace Luch
{

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template< class T, class... Args >
inline SharedPtr<T> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

}
