#include <Husky/SceneV1/Loader/LoaderUtils.h>

namespace Husky::SceneV1::Loader
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
            HUSKY_ASSERT_MSG(false, "Unknown sampler addres mode");
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
            HUSKY_ASSERT_MSG(false, "Unknown mag filter");
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

    int32 CalculateStride(ComponentType componentType, AttributeType attributeType)
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
}
