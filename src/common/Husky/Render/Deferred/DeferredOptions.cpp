#include <Husky/Render/Deferred/DeferredOptions.h>
#include <Husky/Assert.h>

namespace Husky::Render::Deferred
{
    using namespace ShadowMapping;

    void DeferredOptions::SetShadowMappingEnabled(bool enabled)
    {
        if(shadowMappingOptions.has_value() && !enabled)
        {
            shadowMappingOptions.reset();
        }
        else if(!shadowMappingOptions.has_value() && enabled)
        {
            shadowMappingOptions.emplace();
        }
    }

    ShadowMappingOptions& DeferredOptions::GetShadowMappingOptions()
    {
        HUSKY_ASSERT(shadowMappingOptions.has_value());

        return *shadowMappingOptions;
    }
}
