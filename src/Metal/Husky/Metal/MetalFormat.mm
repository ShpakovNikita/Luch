#include <Husky/Metal/MetalFormat.h>
#include <Husky/Graphics/Format.h>
#include <Husky/Assert.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::VertexFormat ToMetalVertexFormat(Format format)
    {
        switch(format)
        {
        case Format::Undefined:
            return mtlpp::VertexFormat::Invalid;
        case Format::R8G8B8A8Unorm:
            return mtlpp::VertexFormat::UChar4Normalized;
        case Format::B8G8R8A8Unorm:
            return mtlpp::VertexFormat::UChar4Normalized_BGRA;
        case Format::R8Sint:
            return mtlpp::VertexFormat::Char;
        case Format::R8Uint:
            return mtlpp::VertexFormat::UChar;
        case Format::R16Sint:
            return mtlpp::VertexFormat::Short;
        case Format::R16Uint:
            return mtlpp::VertexFormat::UShort;
        case Format::R32Uint:
            return mtlpp::VertexFormat::UInt;
        case Format::R32Sfloat:
            return mtlpp::VertexFormat::Float;
        case Format::R8G8Sint:
            return mtlpp::VertexFormat::Char2;
        case Format::R8G8Uint:
            return mtlpp::VertexFormat::UChar2;
        case Format::R16G16Sint:
            return mtlpp::VertexFormat::Short2;
        case Format::R16G16Uint:
            return mtlpp::VertexFormat::UShort2;
        case Format::R32G32Uint:
            return mtlpp::VertexFormat::UInt2;
        case Format::R32G32Sfloat:
            return mtlpp::VertexFormat::Float2;
        case Format::R8G8B8Sint:
            return mtlpp::VertexFormat::Char3;
        case Format::R8G8B8Uint:
            return mtlpp::VertexFormat::UChar3;
        case Format::R16G16B16Sint:
            return mtlpp::VertexFormat::Short3;
        case Format::R16G16B16Uint:
            return mtlpp::VertexFormat::UShort3;
        case Format::R32G32B32Uint:
            return mtlpp::VertexFormat::UInt3;
        case Format::R32G32B32Sfloat:
            return mtlpp::VertexFormat::Float3;
        case Format::R8G8B8A8Sint:
            return mtlpp::VertexFormat::Char4;
        case Format::R8G8B8A8Uint:
            return mtlpp::VertexFormat::UChar4;
        case Format::R16G16B16A16Sint:
            return mtlpp::VertexFormat::Short4;
        case Format::R16G16B16A16Uint:
            return mtlpp::VertexFormat::UShort4;
        case Format::R32G32B32A32Uint:
            return mtlpp::VertexFormat::Int4;
        case Format::R32G32B32A32Sfloat:
            return mtlpp::VertexFormat::Float4;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown vertex format");
            return mtlpp::VertexFormat::Invalid;
        }
    }

    mtlpp::PixelFormat ToMetalPixelFormat(Format format)
    {
        switch(format)
        {
        case Format::Undefined:
            return mtlpp::PixelFormat::Invalid;
        case Format::R8Unorm:
            return mtlpp::PixelFormat::R8Unorm;
        case Format::R8G8Unorm:
            return mtlpp::PixelFormat::RG8Unorm;
        case Format::R8G8Unorm_sRGB:
            return mtlpp::PixelFormat::RG8Unorm_sRGB;
        case Format::R8G8B8Unorm:
            HUSKY_ASSERT_MSG(false, "R8G8B8 is not supported in metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R8G8B8A8Unorm:
            return mtlpp::PixelFormat::RGBA8Unorm;
        case Format::R8G8B8A8Unorm_sRGB:
            return mtlpp::PixelFormat::RGBA8Unorm_sRGB;
        case Format::B8G8R8A8Unorm:
            return mtlpp::PixelFormat::BGRA8Unorm;
        case Format::D16Unorm:
            return mtlpp::PixelFormat::Depth16Unorm;
        case Format::D32Sfloat:
            return mtlpp::PixelFormat::Depth32Float;
        case Format::S8Uint:
            return mtlpp::PixelFormat::Stencil8;
        case Format::D16UnormS8Uint:
            HUSKY_ASSERT_MSG(false, "D16UnormS8Uint is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::D24UnormS8Uint:
            return mtlpp::PixelFormat::Depth24Unorm_Stencil8;
        case Format::D32SfloatS8Uint:
            return mtlpp::PixelFormat::Depth32Float_Stencil8;
        case Format::R8Sint:
            return mtlpp::PixelFormat::R8Sint;
        case Format::R8Uint:
            return mtlpp::PixelFormat::R8Uint;
        case Format::R16Sint:
            return mtlpp::PixelFormat::R16Sint;
        case Format::R16Uint:
            return mtlpp::PixelFormat::R16Uint;
        case Format::R32Uint:
            return mtlpp::PixelFormat::R32Uint;
        case Format::R32Sfloat:
            return mtlpp::PixelFormat::R32Float;
        case Format::R8G8Sint:
            return mtlpp::PixelFormat::RG8Sint;
        case Format::R8G8Uint:
            return mtlpp::PixelFormat::RG8Uint;
        case Format::R16G16Sint:
            return mtlpp::PixelFormat::RG16Sint;
        case Format::R16G16Uint:
            return mtlpp::PixelFormat::RG16Uint;
        case Format::R32G32Uint:
            return mtlpp::PixelFormat::RG32Uint;
        case Format::R32G32Sfloat:
            return mtlpp::PixelFormat::RG32Float;
        case Format::R8G8B8Sint:
            HUSKY_ASSERT_MSG(false, "R8G8B8SInt is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R8G8B8Uint:
            HUSKY_ASSERT_MSG(false, "R8G8B8Uint is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R16G16B16Sint:
            HUSKY_ASSERT_MSG(false, "R16G16B16Sint is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R16G16B16Uint:
            HUSKY_ASSERT_MSG(false, "R16G16B16Uint is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R32G32B32Uint:
            HUSKY_ASSERT_MSG(false, "R32G32B32Uint is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R32G32B32Sfloat:
            HUSKY_ASSERT_MSG(false, "R32G32B32Sfloat is not supported in Metal");
            return mtlpp::PixelFormat::Invalid;
        case Format::R8G8B8A8Sint:
            return mtlpp::PixelFormat::RGBA8Sint;
        case Format::R8G8B8A8Uint:
            return mtlpp::PixelFormat::RGBA8Uint;
        case Format::R16G16B16A16Sint:
            return mtlpp::PixelFormat::RGBA16Sint;
        case Format::R16G16B16A16Uint:
            return mtlpp::PixelFormat::RGBA16Uint;
        case Format::R32G32B32A32Uint:
            return mtlpp::PixelFormat::RGBA32Uint;
        case Format::R32G32B32A32Sfloat:
            return mtlpp::PixelFormat::RGBA32Float;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown format");
            return mtlpp::PixelFormat::Invalid;
        }
    }

}
