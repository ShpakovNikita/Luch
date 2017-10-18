#pragma once

#include <Husky/Types.h>
#include <Husky/VectorTypes.h>

namespace Husky
{

constexpr int32 InvalidMaterialIndex = -1;

#pragma pack(push, 1)
struct Vertex
{
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    int32 materialIndex;
    
    friend bool operator==(const Vertex& lhs, const Vertex& rhs)
    {
        return lhs.position == rhs.position
            && lhs.normal == rhs.normal
            && lhs.texCoord == rhs.texCoord
            && lhs.materialIndex == rhs.materialIndex;
    }

    friend bool operator!=(const Vertex& lhs, const Vertex& rhs)
    {
        return !(lhs == rhs);
    }
};
#pragma pack(pop)

}

//namespace std
//{
//template<> struct hash<Husky::Vertex>
//{
//    typedef Husky::Vertex argument_type;
//    typedef size_t result_type;
//    result_type operator()(argument_type const& s) const
//    {
//        hash<Husky::Vec3> vec3Hash;
//        hash<Husky::Vec2> vec2Hash;
//        hash<int> intHash;
//
//        return vec3Hash(s.Position) ^ vec3Hash(s.Normal) ^ vec2Hash(s.TexCoord) ^ intHash(s.MaterialIndex);
//    }
//};
//}