#include <Luch/Render/Deferred/DeferredOptions.h>
#include <Luch/Assert.h>

namespace Luch::Render::Deferred
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

    const ShadowMappingOptions& DeferredOptions::GetShadowMappingOptions() const
    {
        HUSKY_ASSERT(shadowMappingOptions.has_value());

        return *shadowMappingOptions;
    }
}
