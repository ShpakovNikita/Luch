#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/MaterialResources.h>

namespace Luch::Render
{
    using namespace Graphics;

    class MaterialManager
    {
        static constexpr int32 MaxTexturesPerMaterial = 5;
        static constexpr int32 SharedBufferSize = 1024 * 1024;
        static constexpr int32 DescriptorSetCount = 1024;
        static constexpr int32 DescriptorCount = 1024;
    public:
        bool Initialize(GraphicsDevice* device);
        bool Deinitialize();

        bool PrepareMaterial(SceneV1::PbrMaterial* material);
        void UpdateMaterial(SceneV1::PbrMaterial* material);

        MaterialResources* GetResources() { return resources.get(); }
    private:
        GraphicsDevice* device = nullptr;
        UniquePtr<MaterialResources> resources;
    };
}
