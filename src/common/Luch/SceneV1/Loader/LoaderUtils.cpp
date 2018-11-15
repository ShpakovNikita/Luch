#include <Luch/SceneV1/Loader/LoaderUtils.h>
#include <cmath>

namespace Luch::SceneV1::Loader
{
    using namespace Graphics;

    SamplerAddressMode ToSamplerAddresMode(glTF::WrapMode mode)
    {
        switch (mode)
        {
        case glTF::WrapMode::ClampToEdge:
            return SamplerAddressMode::ClampToEdge;
        case glTF::WrapMode::Repeat:
            return SamplerAddressMode::Repeat;
        case glTF::WrapMode::MirroredRepeat:
            return SamplerAddressMode::MirrorRepeat;
        default:
            LUCH_ASSERT_MSG(false, "Unknown sampler addres mode");
        }
    }

    SamplerMinMagFilter ToMagFilter(glTF::MagFilter mode)
    {
        switch (mode)
        {
        case glTF::MagFilter::Nearest:
            return SamplerMinMagFilter::Nearest;
        case glTF::MagFilter::Linear:
            return SamplerMinMagFilter::Linear;
        default:
            LUCH_ASSERT_MSG(false, "Unknown mag filter");
        }
    }

    MinFilter ToMinFilter(glTF::MinFilter mode)
    {
        MinFilter filter;

        // https://www.khronos.org/registry/vulkan/specs/1.1-khr-extensions/html/vkspec.html#samplers
        switch (mode)
        {
        case glTF::MinFilter::Linear:
            filter.minFilter = SamplerMinMagFilter::Linear;
            filter.mipFilter = SamplerMipFilter::Nearest;
            filter.minLod = 0;
            filter.maxLod = 0.25f;
            break;
        case glTF::MinFilter::LinearMipMapLinear:
            filter.minFilter = SamplerMinMagFilter::Linear;
            filter.mipFilter = SamplerMipFilter::Linear;
            break;
        case glTF::MinFilter::LinearMipMapNearest:
            filter.minFilter = SamplerMinMagFilter::Linear;
            filter.mipFilter = SamplerMipFilter::Nearest;
            break;
        case glTF::MinFilter::Nearest:
            filter.minFilter = SamplerMinMagFilter::Nearest;
            filter.mipFilter = SamplerMipFilter::Nearest;
            filter.minLod = 0;
            filter.maxLod = 0.25f;
            break;
        case glTF::MinFilter::NearestMipMapLinear:
            filter.minFilter = SamplerMinMagFilter::Nearest;
            filter.mipFilter = SamplerMipFilter::Linear;
            break;
        case glTF::MinFilter::NearestMipMapNearest:
            filter.minFilter = SamplerMinMagFilter::Nearest;
            filter.mipFilter = SamplerMipFilter::Nearest;
            break;
        }

        return filter;
    }

    int32 CalculateStride(AttributeType attributeType, ComponentType componentType)
    {
        int32 componentSize = 0;
        int32 componentCount = 0;

        switch (componentType)
        {
        case ComponentType::Int8:
        case ComponentType::UInt8:
            componentSize = 1;
            break;
        case ComponentType::Int16:
        case ComponentType::UInt16:
            componentSize = 2;
            break;
        case ComponentType::UInt:
        case ComponentType::Float:
            componentSize = 4;
            break;
        }

        switch (attributeType)
        {
        case AttributeType::Scalar:
            componentCount = 1;
            break;
        case AttributeType::Vec2:
            componentCount = 2;
            break;
        case AttributeType::Vec3:
            componentCount = 3;
            break;
        case AttributeType::Vec4:
            componentCount = 4;
            break;
        case AttributeType::Mat2x2:
            componentCount = 4;
            break;
        case AttributeType::Mat3x3:
            componentCount = 9;
            break;
        case AttributeType::Mat4x4:
            componentCount = 16;
            break;
        }

        return componentSize * componentCount;
    }

    void GenerateTangents(
        glTF::PrimitiveMode mode,
        int32 indexCount,
        uint32* indices,
        Vec3* positions,
        Vec3* normals,
        Vec2* texcoords,
        Vec4* outTangents)
    {
        //LUCH_ASSERT(mode == glTF::PrimitiveMode::Triangles || mode == glTF::PrimitiveMode::TriangleStrip);
        LUCH_ASSERT(mode == glTF::PrimitiveMode::Triangles);
        //int32 verticesCount = mode == glTF::PrimitiveMode::Triangles ? indexCount * 3 : indexCount + 2;
        int32 verticesCount = indexCount / 3;
        LUCH_ASSERT(verticesCount % 3 == 0);

        Vector<Vec3> tangents;
        tangents.resize(verticesCount);
        Vector<Vec3> bitangents;
        bitangents.resize(verticesCount);

        for(int32 i = 0; i < indexCount; i += 3)
        {
            Array<uint32, 3> ti = { indices[i], indices[i + 1], indices[i + 2] };

            Vec3 p0 = positions[ti[0]];
            Vec3 p1 = positions[ti[1]];
            Vec3 p2 = positions[ti[2]];

            Vec2 uv0 = texcoords[ti[0]];
            Vec2 uv1 = texcoords[ti[1]];
            Vec2 uv2 = texcoords[ti[2]];

            Vec3 dp0 = p1 - p0;
            Vec3 dp1 = p2 - p0;

            Vec2 duv0 = uv1 - uv0;
            Vec2 duv1 = uv2 - uv0;

            float32 r = 1.0f / (duv0.x * duv1.y - duv0.y * duv1.x);

            Vec3 t = (duv1.y * dp0 - duv0.y * dp1) * r;
            Vec3 bt = (duv0.x * dp0 - duv1.x * dp1) * r;

            tangents[ti[0]] += t;
            tangents[ti[1]] += t;
            tangents[ti[2]] += t;

            bitangents[ti[0]] += bt;
            bitangents[ti[1]] += bt;
            bitangents[ti[2]] += bt;
        }

        for (int32 i = 0; i < verticesCount; i++)
        {
            Vec3 n = normals[i];
            Vec3 t = tangents[i];

            // Gram-Schmidt orthogonalize
            Vec3 orthoT = glm::normalize((t - n * glm::dot(n, t)));

            auto NxT = glm::cross(n, t);
            auto dot = glm::dot(NxT, bitangents[i]);

            outTangents[i] = Vec4 { orthoT, std::copysign(1, dot) };
        }
    }
}
