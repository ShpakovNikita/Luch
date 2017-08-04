#pragma once

namespace Husky
{

template<typename Container>
std::tuple<uint32, typename Container::const_pointer> VkArrayParam(const Container& container)
{
    // TODO debug check if size fits into uint32
    return std::make_tuple(static_cast<uint32>(container.size()), container.data());
}

}
