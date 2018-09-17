#include <Husky/SceneV1/Loader/LoaderUtils.h>

namespace Husky::SceneV1::Loader
{
    vk::SamplerAddressMode ToVkSamplerAddresMode(glTF::WrapMode mode)
    {
        switch (mode)
        {
        case glTF::WrapMode::ClampToEdge:
            return vk::SamplerAddressMode::eClampToEdge;
        case glTF::WrapMode::Repeat:
            return vk::SamplerAddressMode::eRepeat;
        case glTF::WrapMode::MirroredRepeat:
            return vk::SamplerAddressMode::eMirroredRepeat;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown sampler addres mode");
        }
    }

    vk::Filter ToVkMagFilter(glTF::MagFilter mode)
    {
        switch (mode)
        {
        case glTF::MagFilter::Nearest:
            return vk::Filter::eNearest;
        case glTF::MagFilter::Linear:
            return vk::Filter::eLinear;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown mag filter");
        }
    }

    
    MinFilter ToVkMinFilter(glTF::MinFilter mode)
    {
        MinFilter filter;

        // https://www.khronos.org/registry/vulkan/specs/1.1-khr-extensions/html/vkspec.html#samplers
        switch (mode)
        {
        case glTF::MinFilter::Linear:
            filter.minFilter = vk::Filter::eLinear;
            filter.mipmapMode = vk::SamplerMipmapMode::eNearest;
            filter.minLod = 0;
            filter.maxLod = 0.25f;
            break;
        case glTF::MinFilter::LinearMipMapLinear:
            filter.minFilter = vk::Filter::eLinear;
            filter.mipmapMode = vk::SamplerMipmapMode::eLinear;
            break;
        case glTF::MinFilter::LinearMipMapNearest:
            filter.minFilter = vk::Filter::eLinear;
            filter.mipmapMode = vk::SamplerMipmapMode::eNearest;
            break;
        case glTF::MinFilter::Nearest:
            filter.minFilter = vk::Filter::eNearest;
            filter.mipmapMode = vk::SamplerMipmapMode::eNearest;
            filter.minLod = 0;
            filter.maxLod = 0.25f;
            break;
        case glTF::MinFilter::NearestMipMapLinear:
            filter.minFilter = vk::Filter::eNearest;
            filter.mipmapMode = vk::SamplerMipmapMode::eLinear;
            break;
        case glTF::MinFilter::NearestMipMapNearest:
            filter.minFilter = vk::Filter::eNearest;
            filter.mipmapMode = vk::SamplerMipmapMode::eNearest;
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
