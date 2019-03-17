#pragma once

namespace Luch::Render::Passes::IBL
{
    class EnvironmentCubemapRenderPass;
    class DiffuseIlluminanceRenderPass;
    class SpecularBRDFRenderPass;
    class SpecularReflectionRenderPass;

    struct EnvironmentCubemapPersistentContext;
    struct EnvironmentCubemapTransientContext;
    struct DiffuseIlluminancePersistentContext;
    struct DiffuseIlluminanceTransientContext;
    struct SpecularBRDFPersistentContext;
    struct SpecularBRDFTransientContext;
    struct SpecularReflectionPersistentContext;
    struct SpecularReflectionTransientContext;
}
