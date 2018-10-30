#pragma once

#include <Husky/Types.h>
#include <Husky/Render/Deferred/DeferredForwards.h>
#include <Husky/Render/Deferred/ShadowMapping/ShadowMappingOptions.h>

namespace Husky::Render::Deferred
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
