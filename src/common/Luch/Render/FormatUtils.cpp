#include <Luch/Render/FormatUtils.h>
#include <Luch/Assert.h>

namespace Luch::Render::FormatUtils
{
    using namespace Graphics;

    Format GetSRGBFormat(Format format)
    {
        switch(format)
        {
        case Format::R8Unorm:
            return Format::R8Unorm_sRGB;
        case Format::RG8Unorm:
            return Format::RG8Unorm_sRGB;
        case Format::RGB8Unorm:
            return Format::RGB8Unorm_sRGB;
        case Format::RGBA8Unorm:
            return Format::RGBA8Unorm_sRGB;
        default:
            LUCH_ASSERT_MSG(false, "Unknown format");
            return Format::Undefined;
        };
    }
}
