#pragma once

#include <Luch/Types.h>
#include <Luch/Render/Deferred/DeferredForwards.h>
#include <Luch/Render/Deferred/ShadowMapping/ShadowMappingOptions.h>

namespace Luch::Render::Deferred
{
    using namespace ShadowMapping;

    class DeferredOptions
    {
    public:
        // Shadow mapping
        bool IsShadowMappingEnabled() const { return shadowMappingOptions.has_value(); }
        void SetShadowMappingEnabled(bool enabled);

        ShadowMappingOptions& GetShadowMappingOptions();
        const ShadowMappingOptions& GetShadowMappingOptions() const;
    private:
        Optional<ShadowMappingOptions> shadowMappingOptions;
    };
}
