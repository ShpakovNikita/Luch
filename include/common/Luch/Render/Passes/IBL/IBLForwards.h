#pragma once

namespace Luch::Render::Passes::IBL
{
    class EnvironmentCubemapRenderPass;
    class DiffuseIrradianceRenderPass;
    class SpecularBRDFRenderPass;
    class SpecularReflectionRenderPass;

    struct EnvironmentCubemapPersistentContext;
    struct EnvironmentCubemapTransientContext;
    struct DiffuseIrradiancePersistentContext;
    struct DiffuseIrradianceTransientContext;
    struct SpecularBRDFPersistentContext;
    struct SpecularBRDFTransientContext;
    struct SpecularReflectionPersistentContext;
    struct SpecularReflectionTransientContext;
}
