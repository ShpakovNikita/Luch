#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/GraphicsPipelineStateCreateInfo.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Render/Common.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/ResultValue.h>

namespace Luch::Render
{
    using namespace Graphics;

    namespace RenderUtils
    {
        MaterialUniform GetMaterialUniform(SceneV1::PbrMaterial *material);
        CameraUniform GetCameraUniform(SceneV1::Camera *camera, const Mat4x4& transform);
        LightUniform GetLightUniform(SceneV1::Light* light, const Mat4x4& worldTransform);
        Rect3f CombineAABB(const Rect3f& aabb1, const Rect3f& aabb2);
        Rect3f TransformAABB(const Rect3f& aabb, const Mat4x4& worldTransform);

        ResultValue<bool, RefPtr<ShaderLibrary>> CreateShaderLibrary(
            GraphicsDevice* device,
            const String& includeDir,
            const String& dir,
            const String& filename,
            const UnorderedMap<String, Variant<int32, String>>& defines);

        ResultValue<bool, String> SubstituteIncludes(
            const String& includeDir,
            String source);

        void SubmitCommandLists(
            const RefPtr<CommandQueue>& queue,
            const RefPtrVector<CommandList>& commandLists,
            const std::function<void()> completedHandler = {});

        GraphicsPipelineVertexInputStateCreateInfo GetPrimitiveVertexInputStateCreateInfo(
            SceneV1::Primitive* primitive);

        void AddPrimitiveVertexShaderDefines(SceneV1::Primitive* primitive, ShaderDefines& shaderDefines);
        void AddMaterialShaderDefines(SceneV1::PbrMaterial* material, ShaderDefines& shaderDefines);
    }
}
