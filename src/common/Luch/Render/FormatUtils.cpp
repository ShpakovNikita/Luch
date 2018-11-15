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
        case Format::R8G8Unorm:
            return Format::R8G8Unorm_sRGB;
        case Format::R8G8B8Unorm:
            return Format::R8G8B8Unorm_sRGB;
        case Format::R8G8B8A8Unorm:
            return Format::R8G8B8A8Unorm_sRGB;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown format");
            return Format::Undefined;
        };
    }
}
