#pragma once

#include <Luch/Types.h>
#include <Luch/Assert.h>
#include <Luch/ResultValue.h>

namespace Luch::Render::Graph
{
    enum class TopologicalSortResult
    {
        Success,
        Cycle,
    };

    ResultValue<TopologicalSortResult, Vector<int32>> TopologicalOrder(
        int32 vertexCount,
        const UnorderedMultimap<int32, int32>& edges)
    {
        enum class Color
        {
            White,
            Gray,
            Black,
        };

        Vector<Color> colors { static_cast<size_t>(vertexCount), Color::White };
        Stack<int32> stack;

        std::function<bool(int32)> dfs;
        dfs = [&](int32 vertex)
        {
            if(colors[vertex] == Color::Gray)
            {
                return true;
            }

            if(colors[vertex] == Color::Black)
            {
                return false;
            }

            colors[vertex] = Color::Gray;
            auto its = edges.equal_range(vertex);
            for(auto it = its.first; it != its.second; ++it)
            {
                if(dfs(it->second))
                {
                    return true;
                }
            }
            stack.push(vertex);
            colors[vertex] = Color::Black;
            return false;
        };

        for(int32 i = 0; i < vertexCount; i++)
        {
            bool cycle = dfs(i);
            if(cycle)
            {
                return { TopologicalSortResult::Cycle };
            }
        }

        Vector<int32> result;
        result.reserve(vertexCount);
        while(!stack.empty())
        {
            result.push_back(stack.top());
            stack.pop();
        }

        return { TopologicalSortResult::Success, std::move(result) };
    }
}